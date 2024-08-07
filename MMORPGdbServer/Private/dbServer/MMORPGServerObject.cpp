#include "MMORPGServerObject.h"
#include "Blueprint/SimpleMysqlObject.h"
#include "SimpleMySQLibrary.h"
#include "MysqlConfig.h"
#include "Log/MMORPGdbServerLog.h"
#include "Protocol/LoginProtocol.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "Protocol/HallProtocol.h"
#include "Protocol/ServerProtocol.h"

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
		`head_size` double(11,4) DEFAULT '0.00',\
		`chest_size` double(11,4) DEFAULT '0.00',\
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
			TArray<FString> InIDs;
			if (GetCharacterIDsByUserMate(InUserID, InIDs))
			{
				GetSerialString(TEXT(","), InIDs, IDs);
			}

			FCharacterAppearacnce CharacterAppearances;
			//拿到角色数据
			if (!IDs.IsEmpty())
			{
				FString SQL = FString::Printf(TEXT("SELECT * FROM mmorpg_characters_ca WHERE id in(%s); "), *IDs);
				TArray<FSimpleMysqlResult> Result;
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
							if (FString* InHeadSize = Temp.Rows.Find(TEXT("head_size")))
							{
								InLast.HeadSize = FCString::Atof(**InHeadSize);
							}
							if (FString* InChestSize = Temp.Rows.Find(TEXT("chest_size")))
							{
								InLast.ChestSize = FCString::Atof(**InChestSize);
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
						SQL = FString::Printf(TEXT("INSERT INTO mmorpg_characters_ca\
											(mmorpg_name,mmorpg_date,mmorpg_slot,leg_size,waist_size,arm_size,head_size,chest_size) \
											VALUES(\"%s\",\"%s\",%i,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf);"), 
											*CA.Name, *CA.Date, CA.SlotPosition, CA.LegSize, CA.WaistSize, CA.ArmSize, CA.HeadSize, CA.ChestSize);
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
	case SP_PlayerRegistInfoRequests:
	{
		//收到中心服务器发来的玩家注册信息请求5
		int32 InUserID = INDEX_NONE;
		int32 SlotID = INDEX_NONE;
		FSimpleAddrInfo GateAddrInfo;
		FSimpleAddrInfo CenterAddrInfo;
		SIMPLE_PROTOCOLS_RECEIVE(SP_PlayerRegistInfoRequests, InUserID, SlotID, GateAddrInfo, CenterAddrInfo);
		UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_PlayerRegistInfoRequests]"));

		if (InUserID != INDEX_NONE && SlotID != INDEX_NONE)
		{
			FString UserInfoJson;
			FString SlotInfoJson;
			if (GetUserInfo(InUserID, UserInfoJson) && GetSlotCAInfo(InUserID, SlotID, SlotInfoJson))
			{
				//向中心服务器发送玩家注册信息成功回调6
				SIMPLE_PROTOCOLS_SEND(SP_PlayerRegistInfoResponses, UserInfoJson, SlotInfoJson, GateAddrInfo, CenterAddrInfo);
				UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_PlayerRegistInfoResponses]"));
			}
			else
			{
				UserInfoJson = TEXT("[]");
				SlotInfoJson = TEXT("[]");
				SIMPLE_PROTOCOLS_SEND(SP_PlayerRegistInfoResponses, UserInfoJson, SlotInfoJson, GateAddrInfo, CenterAddrInfo);
			}
		}
		
		break;
	}
	case SP_DeleteCharacterRequests:
	{
		//收到删除角色的请求
		int32 InUserID = INDEX_NONE;
		FSimpleAddrInfo AddrInfo;
		int32 SlotID = INDEX_NONE;

		SIMPLE_PROTOCOLS_RECEIVE(SP_DeleteCharacterRequests, InUserID, SlotID, AddrInfo);
		if (InUserID != INDEX_NONE && SlotID != INDEX_NONE)
		{
			///数据库
			//先从元数据中拿到用户数据
			FString SQL = FString::Printf(TEXT("SELECT meta_value FROM wp_usermeta WHERE user_id = %i and meta_key = \"character_ca\";"), InUserID);
			TArray<FSimpleMysqlResult> Result;
			FString IDs;
			if (Get(SQL, Result))
			{
				if (Result.Num() > 0)
				{
					for (auto& Temp : Result)
					{
						if (FString* InMetaValue = Temp.Rows.Find(TEXT("meta_value")))
						{
							IDs = InMetaValue->Replace(TEXT("|"), TEXT(","));
						}
					}
				}
			}
			//获取准备移除的数据ID
			int32 SuccessDeleteCount = 0;
			SQL.Empty();
			SQL = FString::Printf(TEXT("SELECT id FROM mmorpg_characters_ca WHERE id in (%s) and mmorpg_slot=%i;"), *IDs, SlotID);

			FString RemoveID;
			Result.Empty();
			if (Get(SQL, Result))
			{
				if (Result.Num() > 0)
				{
					for (auto& Temp : Result)
					{
						if (FString* InIDValue = Temp.Rows.Find(TEXT("id")))
						{
							RemoveID = *InIDValue;
							SuccessDeleteCount++;
						}
					}
				}
			}

			//删除Slot对应的对象
			if (!IDs.IsEmpty())
			{
				SQL.Empty();
				SQL = FString::Printf(TEXT("DELETE FROM mmorpg_characters_ca WHERE ID in (%s) and mmorpg_slot=%i;"),*IDs,SlotID);
				if (Post(SQL))
				{
					SuccessDeleteCount++;
				}
			}

			//更新元数据表
			if (SuccessDeleteCount > 1)
			{
				TArray<FString> IDArray;
				IDs.ParseIntoArray(IDArray, TEXT(","));

				//移除ID
				IDArray.Remove(RemoveID);

				IDs.Empty();
				for (auto& Temp :IDArray)
				{
					IDs += (Temp + TEXT("|"));
				}
				IDs.RemoveFromEnd("|");
				SQL.Empty();
				SQL = FString::Printf(TEXT("UPDATE wp_usermeta SET meta_value=\"%s\" WHERE meta_key=\"character_ca\" and user_id=%i;"), *IDs, InUserID);
				if (Post(SQL))
				{
					SuccessDeleteCount++;
				}
			}

			//发送删除角色回调
			SIMPLE_PROTOCOLS_SEND(SP_DeleteCharacterResponses, InUserID, SlotID, SuccessDeleteCount, AddrInfo);

			UE_LOG(LogMMORPGdbServer, Display, TEXT("[SP_DeleteCharacterResponses]"));
		}
		break;
	}
	case SP_EditorCharacterRequests:
	{
		//收到编辑角色的请求
		int32 InUserID = INDEX_NONE;
		FString CAJson;
		FSimpleAddrInfo AddrInfo;

		SIMPLE_PROTOCOLS_RECEIVE(SP_EditorCharacterRequests, InUserID, CAJson, AddrInfo);
		if (InUserID > 0 && CAJson.Len() > 0)
		{
			//将角色Json解析成角色信息
			FMMORPGCharacterAppearance CA;
			NetDataAnalysis::StringToFCharacterAppearacnce(CAJson, CA);
			if (CA.SlotPosition != INDEX_NONE)
			{
				///数据库
				//先从元数据中拿到用户数据
				FString SQL = FString::Printf(TEXT("SELECT meta_value FROM wp_usermeta WHERE meta_key=\"character_ca\" and user_id=%i;"), InUserID);
				TArray<FSimpleMysqlResult> Result;
				FString IDs;
				if (Get(SQL, Result))
				{
					if (Result.Num() > 0)
					{
						for (auto& Temp : Result)
						{
							if (FString* InMetaValue = Temp.Rows.Find(TEXT("meta_value")))
							{
								IDs = InMetaValue->Replace(TEXT("|"),TEXT(","));
							}
						}
					}
				}
				//获取要更新的角色ID
				SQL.Empty();
				SQL = FString::Printf(TEXT("SELECT id FROM mmorpg_characters_ca WHERE mmorpg_slot=%i and id in (%s)"), CA.SlotPosition, *IDs);
				Result.Empty();
				int32 UpdateID = INDEX_NONE;
				if (Get(SQL, Result))
				{
					if (Result.Num() > 0)
					{
						for (auto& Temp : Result)
						{
							if (FString* InidValue = Temp.Rows.Find(TEXT("id")))
							{
								UpdateID = FCString::Atoi(**InidValue);
							}
						}
					}
				}

				//更新角色数据
				if (UpdateID != INDEX_NONE)
				{
					SQL.Empty();
					SQL = FString::Printf(
						TEXT("UPDATE mmorpg_characters_ca \
							SET mmorpg_name=\"%s\" ,mmorpg_date=\"%s\",mmorpg_slot=%i,\
							leg_size=%.2lf,waist_size=%.2lf,arm_size=%.2lf,head_size=%.2lf,chest_size=%.2lf WHERE id=%i"),
						*CA.Name, *CA.Date, CA.SlotPosition,
						CA.LegSize, CA.WaistSize, CA.ArmSize,CA.HeadSize,CA.ChestSize, UpdateID);

					bool bUpdateSucceeded = false;

					//发送编辑角色回调
					if (Post(SQL))
					{
						bUpdateSucceeded = true;
						SIMPLE_PROTOCOLS_SEND(SP_EditorCharacterResponses, bUpdateSucceeded, AddrInfo);
						UE_LOG(LogMMORPGdbServer, Display, TEXT("SP_EditorCharacterResponses true"));
					}
					else
					{
						SIMPLE_PROTOCOLS_SEND(SP_EditorCharacterResponses, bUpdateSucceeded, AddrInfo);
						UE_LOG(LogMMORPGdbServer, Display, TEXT("SP_EditorCharacterResponses false"));
					}
				}
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

bool UMMORPGServerObject::GetUserInfo(int32 InUserID, FString& OutJsonString)
{
	FMMORPGUserData UserData;
	UserData.ID = InUserID;
	//访问数据库
	FString SQL = FString::Printf(TEXT("SELECT user_login,user_email,user_url,display_name FROM wp_users WHERE ID=%i;"), InUserID);
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
	NetDataAnalysis::UserDataToString(UserData, OutJsonString);

	return OutJsonString.Len() > 0;
}

bool UMMORPGServerObject::GetSlotCAInfo(int32 InUserID, int32 InSlotID, FString& OutJsonString)
{
	TArray<FString> IDs;
	if (GetCharacterIDsByUserMate(InUserID, IDs))
	{
		FString IDString;
		GetSerialString(TEXT(","), IDs, IDString);

		//拿到角色数据
		FString SQL = FString::Printf(TEXT("SELECT * FROM mmorpg_characters_ca WHERE id in(%s) and mmorpg_slot=%i; "), *IDString, InSlotID);
		TArray<FSimpleMysqlResult> Result;
		if (Get(SQL, Result))
		{
			if (Result.Num() > 0)
			{
				FMMORPGCharacterAppearance CA;
				//解析数据库返回的数据,组装成角色列表
				for (auto& Temp : Result)
				{
					if (FString* InName = Temp.Rows.Find(TEXT("mmorpg_name")))
					{
						CA.Name = *InName;
					}
					if (FString* InDate = Temp.Rows.Find(TEXT("mmorpg_date")))
					{
						CA.Date = *InDate;
					}
					if (FString* InSlotPos = Temp.Rows.Find(TEXT("mmorpg_slot")))
					{
						CA.SlotPosition = FCString::Atoi(**InSlotPos);
					}
					if (FString* InLegSize = Temp.Rows.Find(TEXT("leg_size")))
					{
						CA.LegSize = FCString::Atof(**InLegSize);
					}
					if (FString* InWaistSize = Temp.Rows.Find(TEXT("waist_size")))
					{
						CA.WaistSize = FCString::Atof(**InWaistSize);
					}
					if (FString* InArmSize = Temp.Rows.Find(TEXT("arm_size")))
					{
						CA.ArmSize = FCString::Atof(**InArmSize);
					}
					if (FString* InHeadSize = Temp.Rows.Find(TEXT("head_size")))
					{
						CA.HeadSize = FCString::Atof(**InHeadSize);
					}
					if (FString* InChestSize = Temp.Rows.Find(TEXT("chest_size")))
					{
						CA.ChestSize = FCString::Atof(**InChestSize);
					}
				}

				//将角色数据转成Json
				NetDataAnalysis::CharacterAppearacnceToString(CA, OutJsonString);

				return !OutJsonString.IsEmpty();
			}
		}
	}

	return false;
}

bool UMMORPGServerObject::GetCharacterIDsByUserMate(int32 InUserID, TArray<FString>& OutIDs)
{
	FString SQL = FString::Printf(TEXT("SELECT meta_value FROM wp_usermeta WHERE user_id = %i and meta_key = \"character_ca\";"), InUserID);
	TArray<FSimpleMysqlResult> Result;
	if (Get(SQL, Result))
	{
		if (Result.Num() > 0)
		{
			for (auto& MetaTemp : Result)
			{
				if (FString* InMetaValue = MetaTemp.Rows.Find(TEXT("meta_value")))
				{
					InMetaValue->ParseIntoArray(OutIDs, TEXT("|"));
				}
			}
		}
		return true;
	}
	return OutIDs.Num() > 0;
}

void UMMORPGServerObject::GetSerialString(TCHAR* InPrefix, const TArray<FString>& InStrings, FString& OutString)
{
	OutString.Empty();
	for (auto& Temp : InStrings)
	{
		OutString += Temp + InPrefix;
	}
	OutString.RemoveFromEnd(InPrefix);
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
					if (GetUserInfo(UserID, String))
					{
						UE_LOG(LogMMORPGdbServer, Display, TEXT("MMORPGdbServer login success"));
						//向登录服务器发送登录回调，告知登录成功
						ELoginType Type = ELoginType::LOGIN_SUCCESS;
						SIMPLE_PROTOCOLS_SEND(SP_LoginResponses, AddrInfo, Type, String);
					}
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

