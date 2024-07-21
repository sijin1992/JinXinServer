#include "MMORPGServerObject.h"
#include "Blueprint/SimpleMysqlObject.h"
#include "SimpleMySQLibrary.h"
#include "MysqlConfig.h"
#include "Log/MMORPGdbServerLog.h"
#include "Protocol/LoginProtocol.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "Protocol/HallProtocol.h"

void UMMORPGServerObject::Init()
{
	Super::Init();

	MysqlObjectRead = USimpleMySQLLibrary::CreateMysqlObject(
		NULL,
		FSimpleMysqlConfig::Get()->GetInfo().User,
		FSimpleMysqlConfig::Get()->GetInfo().Host,
		FSimpleMysqlConfig::Get()->GetInfo().Pawd,
		FSimpleMysqlConfig::Get()->GetInfo().DB,
		FSimpleMysqlConfig::Get()->GetInfo().Port,
		FSimpleMysqlConfig::Get()->GetInfo().ClientFlags		
		);

	MysqlObjectWrite = USimpleMySQLLibrary::CreateMysqlObject(
		NULL,
		FSimpleMysqlConfig::Get()->GetInfo().User,
		FSimpleMysqlConfig::Get()->GetInfo().Host,
		FSimpleMysqlConfig::Get()->GetInfo().Pawd,
		FSimpleMysqlConfig::Get()->GetInfo().DB,
		FSimpleMysqlConfig::Get()->GetInfo().Port,
		FSimpleMysqlConfig::Get()->GetInfo().ClientFlags
	);

	/*
	//测试Mysql架构
	FString SQL = "SELECT * FROM wp_users WHERE ID = 1";
	TArray<FSimpleMysqlResult> Results;
	Get(SQL, Results);
	for (auto &Temp : Results)
	{
	}
	*/

	//double(2,2):000.00
	//初始化
	FString Create_mmorpg_characters_ca_SQL = 
		TEXT("CREATE TABLE IF NOT EXISTS `mmorpg_characters_ca`(\
		`id` INT UNSIGNED AUTO_INCREMENT,\
		`mmorpg_name` VARCHAR(100) NOT NULL,\
		`mmorpg_date` VARCHAR(100) NOT NULL,\
		`mmorpg_slot` INT,\
		`leg_size` double(11,4) DEFAULT '0.00',\
		`waist_size` double(11,4) DEFAULT '0.00',\
		`arm_size` double(11,4) DEFAULT '0.00',\
		PRIMARY KEY(`id`)\
		) ENGINE = INNODB DEFAULT CHARSET = utf8; ");

	if (!Post(Create_mmorpg_characters_ca_SQL))
	{
		UE_LOG(LogMMORPGdbServer, Error, TEXT("create table mmorpg_characters_ca failed."));
	}
}

void UMMORPGServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGServerObject::Close()
{
	Super::Close();

	if (MysqlObjectRead)
	{
		MysqlObjectRead->ConditionalBeginDestroy();//条件删除
		MysqlObjectRead = nullptr;
	}
	if (MysqlObjectWrite)
	{
		MysqlObjectWrite->ConditionalBeginDestroy();
		MysqlObjectWrite = nullptr;
	}
}

void UMMORPGServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_LoginRequests:
	{
		//接收到登录服务器转发的登录请求协议
		FString AccountString;
		FString PasswordString;
		FSimpleAddrInfo AddrInfo;

		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginRequests, AccountString, PasswordString, AddrInfo)

			FString String = TEXT("[]");//发送协议时的自定义参数
		//访问数据库
		FString SQL = FString::Printf(TEXT("SELECT ID,user_pass FROM wp_users WHERE user_login = '%s' or user_email = '%s';"), *AccountString, *AccountString);
		TArray<FSimpleMysqlResult> Result;
		if (Get(SQL, Result))
		{
			if (Result.Num() > 0)
			{
				//解析数据库返回的数据
				for (auto& Temp : Result)
				{
					int32 UserID = 0;
					if (FString* IDString = Temp.Rows.Find(TEXT("ID")))
					{
						UserID = FCString::Atoi(**IDString);
					}

					if (FString* UserPass = Temp.Rows.Find(TEXT("user_pass")))
					{
						///通过HTTP协议发送Post请求给worldpress的php，来验证密码
						//拼接URL参数
						FString Parma = FString::Printf(TEXT("EncryptedPassword=%s&Password=%s&IP=%i&Port=%i&Channel=%s&UserID=%i"),
							**UserPass,
							*PasswordString,
							AddrInfo.Addr.IP,
							AddrInfo.Addr.Port,
							*AddrInfo.ChannelID.ToString(),
							UserID);
						//绑定Post请求的回调代理
						FSimpleHTTPResponseDelegate Delegate;
						Delegate.SimpleCompleteDelegate.BindUObject(this, &UMMORPGServerObject::CheckPasswordResult);
						//发送Post请求
						FString WpIP = FSimpleNetGlobalInfo::Get()->GetInfo().PublicIP;//获取公网IP
						SIMPLE_HTTP.PostRequest(
							*FString::Printf(TEXT("http://%s/wp/wp-content/plugins/SimplePasswordVerification/SimplePasswordVerification.php"), *WpIP),
							*Parma,
							Delegate);
					}
				}
			}
			else
			{
				//向登录服务器发送登录回调
				ELoginType Type = ELoginType::LOGIN_ACCOUNT_WRONG;
				SIMPLE_PROTOCOLS_SEND(SP_LoginResponses, AddrInfo, Type, String);
			}
		}
		else
		{
			//向登录服务器发送登录回调
			ELoginType Type = ELoginType::LOGIN_DB_SERVER_ERROR;
			SIMPLE_PROTOCOLS_SEND(SP_LoginResponses, AddrInfo, Type, String);
		}

		UE_LOG(LogMMORPGdbServer, Display, TEXT("AccountString=%s,PasswordString=%s"), *AccountString, *PasswordString);

		break;
	}
	case SP_CharacterAppearanceRequests:
	{
		//收到GateServer发送的角色信息请求
		int32 InUserID = INDEX_NONE;
		FSimpleAddrInfo AddrInfo;
		
		SIMPLE_PROTOCOLS_RECEIVE(SP_CharacterAppearanceRequests, InUserID, AddrInfo);
		if (InUserID > 0)
		{
			///数据库查询	
			//先拿到用户元数据
			FString IDs;
			FString SQL = FString::Printf(TEXT("SELECT meta_value FROM wp_usermeta WHERE user_id = %i and meta_key = \"character_ca\";"), InUserID);
			TArray<FSimpleMysqlResult> Result;
			if (Get(SQL, Result))
			{
				if (Result.Num() > 0)
				{
					//解析数据库返回的数据,组装成角色ID字符串："1, 2, 3, 4, 5"
					for (auto& MetaTemp : Result)
					{
						if (FString* InMetaValue = MetaTemp.Rows.Find(TEXT("meta_value")))
						{
							TArray<FString> Arrays;
							InMetaValue->ParseIntoArray(Arrays, TEXT("|"));
							for (auto& TempID : Arrays)
							{
								IDs += TempID + TEXT(",");
							}
							IDs.RemoveFromEnd(TEXT(","));
						}
					}
				}
			}

			FCharacterAppearacnce CharacterAppearances;
			//拿到角色数据
			if (!IDs.IsEmpty())
			{
				SQL.Empty();
				SQL = FString::Printf(TEXT("SELECT * FROM mmorpg_characters_ca WHERE id in(%s); "), *IDs);
				Result.Empty();
				if (Get(SQL, Result))
				{
					if (Result.Num() > 0)
					{
						//解析数据库返回的数据,组装成角色列表
						for (auto& Temp : Result)
						{
							CharacterAppearances.Add(FMMORPGCharacterAppearance());
							FMMORPGCharacterAppearance& InLast = CharacterAppearances.Last();
							if (FString* InName = Temp.Rows.Find(TEXT("mmorpg_name")))
							{
								InLast.Name = *InName;
							}
							if (FString* InDate = Temp.Rows.Find(TEXT("mmorpg_date")))
							{
								InLast.Date = *InDate;
							}
							if (FString* InSlotPos = Temp.Rows.Find(TEXT("mmorpg_slot")))
							{
								InLast.SlotPosition = FCString::Atoi(**InSlotPos);
							}
							if (FString* InLegSize = Temp.Rows.Find(TEXT("leg_size")))
							{
								InLast.LegSize = FCString::Atof(**InLegSize);
							}
							if (FString* InWaistSize = Temp.Rows.Find(TEXT("waist_size")))
							{
								InLast.WaistSize = FCString::Atof(**InWaistSize);
							}
							if (FString* InArmSize = Temp.Rows.Find(TEXT("arm_size")))
							{
								InLast.ArmSize = FCString::Atof(**InArmSize);
							}
						}
					}
				}
			}

			//将角色数据转成Json
			FString JsonString;
			NetDataAnalysis::CharacterAppearacnceToString(CharacterAppearances, JsonString);

			//发送角色请求回调
			SIMPLE_PROTOCOLS_SEND(SP_CharacterAppearanceResponses, AddrInfo, JsonString);

			UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_CharacterAppearanceResponses]"));
		}

		break;
	}
	case SP_CheckCharacterNameRequests:
	{
		//收到检查角色名字的请求
		int32 InUserID = INDEX_NONE;
		FString CharacterName;
		FSimpleAddrInfo AddrInfo;

		SIMPLE_PROTOCOLS_RECEIVE(SP_CheckCharacterNameRequests, InUserID, CharacterName, AddrInfo);

		ECheckNameType CheckNameType = ECheckNameType::UNKNOWN_ERROR;
		if (InUserID > 0)
		{
			CheckNameType = CheckName(CharacterName);
		}

		//发送检查角色名字回调
		SIMPLE_PROTOCOLS_SEND(SP_CheckCharacterNameResponses, CheckNameType, AddrInfo);

		UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_CheckCharacterNameResponses]"));
		break;
	}
	case SP_CreateCharacterRequests:
	{
		//收到创建角色的请求
		int32 InUserID = INDEX_NONE;
		FString CAJson;
		FSimpleAddrInfo AddrInfo;

		SIMPLE_PROTOCOLS_RECEIVE(SP_CreateCharacterRequests, InUserID, CAJson, AddrInfo);
		if (InUserID > 0)
		{
			//将角色Json解析成角色信息
			FMMORPGCharacterAppearance CA;
			NetDataAnalysis::StringToFCharacterAppearacnce(CAJson, CA);
			if (CA.SlotPosition != INDEX_NONE)
			{
				///数据库
				//验证角色名字
				ECheckNameType CheckNameType = CheckName(CA.Name);
				bool bCreateCharacter = false;//是否可以继续创角的步骤
				if (CheckNameType == ECheckNameType::NAME_NOT_EXIST)//角色名不存在才能创角
				{
					//先从元数据中拿到用户数据
					bool bMetaExist = false;//是否已经有元数据了
					TArray<FString> CAIDs;
					FString SQL = FString::Printf(TEXT("SELECT meta_value FROM wp_usermeta WHERE user_id = %i and meta_key = \"character_ca\";"), InUserID);
					TArray<FSimpleMysqlResult> Result;
					if (Get(SQL, Result))
					{
						if (Result.Num() > 0)
						{
							for (auto& Temp : Result)
							{
								if (FString* InMetaValue = Temp.Rows.Find(TEXT("meta_value")))
								{
									InMetaValue->ParseIntoArray(CAIDs, TEXT("|"));
								}
							}
							bMetaExist = true;
						}
						bCreateCharacter = true;
					}
					//插入角色数据
					if (bCreateCharacter)
					{
						SQL.Empty();
						SQL = FString::Printf(TEXT("INSERT INTO mmorpg_characters_ca(mmorpg_name,mmorpg_date,mmorpg_slot,leg_size,waist_size,arm_size) VALUES(\"%s\",\"%s\",%i,%.2lf,%.2lf,%.2lf);"), 
							*CA.Name, *CA.Date, CA.SlotPosition, CA.LegSize, CA.WaistSize, CA.ArmSize);
						if (Post(SQL))
						{
							//插入成功后查询角色名字
							SQL.Empty();
							SQL = FString::Printf(TEXT("SELECT id FROM mmorpg_characters_ca WHERE mmorpg_name = \"%s\";"), *CA.Name);
							Result.Empty();
							if (Get(SQL, Result))
							{
								if (Result.Num() > 0)
								{
									for (auto& Temp : Result)
									{
										if (FString* InIDString = Temp.Rows.Find(TEXT("id")))
										{
											CAIDs.Add(*InIDString);
										}
									}
								}
							}
							else
							{
								bCreateCharacter = false;
							}
						}
						else
						{
							bCreateCharacter = false;
						}
					}
					//更新元数据
					if (bCreateCharacter)
					{
						FString IDString;
						for (auto& Temp : CAIDs) { IDString += Temp + TEXT("|"); }
						IDString.RemoveFromEnd(TEXT("|"));
						SQL.Empty();
						if (bMetaExist)//如果已经有元数据了，就更新
						{
							SQL = FString::Printf(TEXT("UPDATE wp_usermeta SET meta_value=\"%s\" WHERE meta_key=\"character_ca\" and user_id=%i;"), *IDString, InUserID);
						}
						else
						{//如果没有元数据，就插入新的
							SQL = FString::Printf(TEXT("INSERT INTO wp_usermeta(user_id,meta_key,meta_value) VALUES(%i,\"character_ca\",\"%s\");"), InUserID, *IDString);
						}

						if (!Post(SQL))
						{
							bCreateCharacter = false;
						}
					}
				}

				//发送创建角色回调
				SIMPLE_PROTOCOLS_SEND(SP_CreateCharacterResponses, CheckNameType, bCreateCharacter, CAJson, AddrInfo);

				UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_CreateCharacterResponses]"));
			}
		}
		break;
	}
	}
}

ECheckNameType UMMORPGServerObject::CheckName(const FString& InName)
{
	ECheckNameType CheckNameType = ECheckNameType::UNKNOWN_ERROR;
	if (!InName.IsEmpty())
	{
		//数据库
		FString SQL = FString::Printf(TEXT("SELECT id FROM mmorpg_characters_ca WHERE mmorpg_name = \"%s\";"), *InName);
		TArray<FSimpleMysqlResult> Result;
		if (Get(SQL, Result))
		{
			if (Result.Num() > 0)
			{
				CheckNameType = ECheckNameType::NAME_EXIST;
			}
			else
			{
				CheckNameType = ECheckNameType::NAME_NOT_EXIST;
			}
		}
		else
		{
			CheckNameType = ECheckNameType::SERVER_NOT_EXIST;
		}
	}
	return CheckNameType;
}

void UMMORPGServerObject::CheckPasswordResult(const FSimpleHttpRequest& InRequest, const FSimpleHttpResponse& InResponse, bool bLinkSuccessful)
{
	if (bLinkSuccessful)
	{
		//如果验证密码回调成功，则解析数据
		//xx$IP$Port$0
		TArray<FString> Values;
		InResponse.ResponseMessage.ParseIntoArray(Values, TEXT("&"));

		FSimpleAddrInfo AddrInfo;
		uint32 UserID = 0;
		EPasswordVerification PV = EPasswordVerification::VERIFICATION_FAIL;
		if (Values.Num())
		{
			if (Values.IsValidIndex(0))
			{
				UserID = FCString::Atoi(*Values[0]);
			}
			if (Values.IsValidIndex(1))
			{
				AddrInfo.Addr.IP = FCString::Atoi(*Values[1]);
			}
			if (Values.IsValidIndex(2))
			{
				AddrInfo.Addr.Port = FCString::Atoi(*Values[2]);
			}
			if (Values.IsValidIndex(3))
			{
				FGuid::ParseExact(Values[3], EGuidFormats::Digits, AddrInfo.ChannelID);
			}
			if (Values.IsValidIndex(4))
			{
				PV = (EPasswordVerification)FCString::Atoi(*Values[4]);
			}

			FString String = TEXT("[]");//发送协议时的自定义参数

			//如果验证密码成功
			if (PV == VERIFICATION_SUCCESS)
			{
				if (UserID != 0)
				{
					FMMORPGUserData UserData;
					UserData.ID = UserID;
					//访问数据库
					FString SQL = FString::Printf(TEXT("SELECT user_login,user_email,user_url,display_name FROM wp_users WHERE ID=%i;"), UserID);
					TArray<FSimpleMysqlResult> Result;

					if (Get(SQL, Result))
					{
						if (Result.Num() > 0)
						{
							//解析数据库返回的数据
							for (auto& Temp : Result)
							{
								if (FString* InUserLogin = Temp.Rows.Find(TEXT("user_login")))
								{
									UserData.Account = *InUserLogin;
								}
								if (FString* InUserEmail = Temp.Rows.Find(TEXT("user_email")))
								{
									UserData.Email = *InUserEmail;
								}
								//if (FString* InUserUrl = Temp.Rows.Find(TEXT("user_url")))
								//{
								//	UserData.Email = *InUserEmail;
								//}
								if (FString* InDisplayName = Temp.Rows.Find(TEXT("display_name")))
								{
									UserData.Name = *InDisplayName;
								}
							}
						}
					}
					//将数据转成Json
					NetDataAnalysis::UserDataToString(UserData, String);

					UE_LOG(LogMMORPGdbServer, Display, TEXT("MMORPGdbServer login success"));
					//向登录服务器发送登录回调，告知登录成功
					ELoginType Type = ELoginType::LOGIN_SUCCESS;
					SIMPLE_PROTOCOLS_SEND(SP_LoginResponses, AddrInfo, Type, String);
				}
			}
			else
			{
				//向登录服务器发送登录回调，告知验证密码失败
				ELoginType Type = ELoginType::LOGIN_WRONG_PASSWORD;
				SIMPLE_PROTOCOLS_SEND(SP_LoginResponses, AddrInfo, Type, String);
			}
		}
	}
}

bool UMMORPGServerObject::Post(const FString& InSQL)
{
	if (!InSQL.IsEmpty())
	{
		if (MysqlObjectWrite)
		{
			FString ErrMsg;
			USimpleMySQLLibrary::QueryLink(MysqlObjectWrite, InSQL, ErrMsg);//查询语句

			if (ErrMsg.IsEmpty())
			{
				return true;
			}
			else
			{
				//
				UE_LOG(LogMMORPGdbServer, Error, TEXT("MMORPGdbServer Error: Post msg [ %s]"), *ErrMsg);
			}
		}
	}

	return false;
}

bool UMMORPGServerObject::Get(const FString& InSQL, TArray<FSimpleMysqlResult>& Results)
{
	if (!InSQL.IsEmpty())
	{
		if (MysqlObjectRead)
		{
			FSimpleMysqlDebugResult Debug;
			Debug.bPrintToLog = true;//是否打印日志,正式服务器关闭,调试时打开

			FString ErrMsg;
			USimpleMySQLLibrary::QueryLinkResult(
				MysqlObjectRead,
				InSQL,
				Results,
				ErrMsg,
				EMysqlQuerySaveType::STORE_RESULT,//查询时先把数据下载下来再查，还有在线查模式
				Debug);//查询语句并返回

			if (ErrMsg.IsEmpty())
			{
				return true;
			}
			else
			{
				//
				UE_LOG(LogMMORPGdbServer, Error, TEXT("MMORPGdbServer Error: Get msg [ %s]"), *ErrMsg);
			}
		}
	}

	return false;
}

