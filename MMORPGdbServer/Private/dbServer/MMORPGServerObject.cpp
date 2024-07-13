#include "MMORPGServerObject.h"
#include "Blueprint/SimpleMysqlObject.h"
#include "SimpleMySQLibrary.h"
#include "MysqlConfig.h"

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
			}
		}
	}

	return false;
}

