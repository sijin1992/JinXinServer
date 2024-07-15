#include "MMORPGServerObject.h"
#include "Blueprint/SimpleMysqlObject.h"
#include "SimpleMySQLibrary.h"
#include "MysqlConfig.h"
#include "Log/MMORPGdbServerLog.h"
#include "Protocol/LoginProtocol.h"
#include "MMORPGType.h"
#include "Global/SimpleNetGlobalInfo.h"

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
								}
								if (FString* InUserEmail = Temp.Rows.Find(TEXT("user_email")))
								{
								}
								if (FString* InUserUrl = Temp.Rows.Find(TEXT("user_url")))
								{
								}
								if (FString* InDisplayName = Temp.Rows.Find(TEXT("display_name")))
								{
								}
							}
						}
					}

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

