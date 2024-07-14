// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGLoginServer.h"
#include "Log/MMORPGLoginServerLog.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "RequiredProgramMainCPPInclude.h"
#include "LoginServer/MMORPGLoginServerObject.h"

IMPLEMENT_APPLICATION(MMORPGLoginServer, "MMORPGLoginServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGLoginServer, Display, TEXT("MMORPGLoginServer Beginning"));

	//初始化插件
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例
	FSimpleNetManage* LoginServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);

	//注册反射类
	LoginServer->NetworkObjectClass = UMMORPGLoginServerObject::StaticClass();

	//初始化服务器
	if (!LoginServer->Init())
	{
		delete LoginServer;
		UE_LOG(LogMMORPGLoginServer, Error, TEXT("LoginServer Init fail."));
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

		//每帧检测连接
		LoginServer->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FSimpleNetManage::Destroy(LoginServer);
	FEngineLoop::AppExit();
	return 0;
}
