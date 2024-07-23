// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGGateServer.h"
#include "Log/MMORPGGateServerLog.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "ServerList.h"
#include "GateServer/MMORPGGateServerObject.h"
#include "GateServer/MMORPGdbClientObject.h"
#include "GateServer/MMORPGCenterClientObject.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(MMORPGGateServer, "MMORPGGateServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGGateServer, Display, TEXT("MMORPGGateServer Beginning"));
	
	//初始化插件
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例
	GateServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	//创建db客户端
	dbClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	CenterClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	//注册反射类
	GateServer->NetworkObjectClass = UMMORPGGateServerObject::StaticClass();
	dbClient->NetworkObjectClass = UMMORPGdbClientObject::StaticClass();
	CenterClient->NetworkObjectClass = UMMORPGCenterClientObject::StaticClass();

	//初始化服务器
	if (!GateServer->Init())
	{
		delete GateServer;
		UE_LOG(LogMMORPGGateServer, Error, TEXT("GateServer Init fail."));
		return INDEX_NONE;
	}
	//初始化客户端
	if (!dbClient->Init(11221))
	{
		delete dbClient;
		UE_LOG(LogMMORPGGateServer, Error, TEXT("dbClient Init fail."));
		return INDEX_NONE;
	}
	if (!CenterClient->Init(11231))
	{
		delete dbClient;
		UE_LOG(LogMMORPGGateServer, Error, TEXT("dbClient Init fail."));
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
		dbClient->Tick(DeltaSeconds);
		CenterClient->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FSimpleNetManage::Destroy(GateServer);
	FSimpleNetManage::Destroy(dbClient);
	FSimpleNetManage::Destroy(CenterClient);
	FEngineLoop::AppExit();
	return 0;
}
