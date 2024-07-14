// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGGateServer.h"
#include "Log/MMORPGGateServerLog.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(MMORPGGateServer, "MMORPGGateServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGGateServer, Display, TEXT("MMORPGGateServer Beginning"));
	
	//初始化插件
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例
	FSimpleNetManage* GateServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);

	//注册反射类


	//初始化服务器
	if (!GateServer->Init())
	{
		delete GateServer;
		UE_LOG(LogMMORPGGateServer, Error, TEXT("GateServer Init fail."));
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
		GateServer->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FEngineLoop::AppExit();
	return 0;
}
