// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGdbServer.h"
#include "dbServer/MysqlConfig.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "dbServer/MMORPGServerObject.h"
#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMMORPGdbServer, Log, All);

IMPLEMENT_APPLICATION(MMORPGdbServer, "MMORPGdbServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGdbServer, Display, TEXT("Hello World"));

	//初始化配置表
	FSimpleMysqlConfig::Get()->Init();
	//初始化插件相关内容
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例（使用TCP协议的监听服务器）
	FSimpleNetManage* dbServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);

	//注册反射类
	FSimpleChannel::SimpleControllerDelegate.BindLambda([]() ->UClass*
		{
			return UMMORPGServerObject::StaticClass();
		});

	//初始化服务器
	if (!dbServer->Init())
	{
		delete dbServer;
		UE_LOG(LogMMORPGdbServer, Error, TEXT("dbServer Init fail."));
		return INDEX_NONE;
	}

	//Tick检测
	double LastTime = FPlatformTime::Seconds();
	while (!IsEngineExitRequested())
	{
		//休眠
		FPlatformProcess::Sleep(0.03f);

		//计算时间差
		double Now = FPlatformTime::Seconds();
		float DeltaSeconds = Now - LastTime;

		//每帧检测链接
		dbServer->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FSimpleNetManage::Destroy(dbServer);
	FEngineLoop::AppExit();
	return 0;
}
