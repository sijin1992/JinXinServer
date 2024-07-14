// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGLoginServer.h"
#include "Log/MMORPGLoginServerLog.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "RequiredProgramMainCPPInclude.h"
#include "LoginServer/MMORPGLoginServerObject.h"
#include "ServerList.h"
#include "LoginServer/MMORPGdbClientObject.h"
#include "LoginServer/MMORPGGateClientObject.h"

IMPLEMENT_APPLICATION(MMORPGLoginServer, "MMORPGLoginServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGLoginServer, Display, TEXT("MMORPGLoginServer Beginning"));

	//初始化插件
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例
	LoginServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	//创建客户端实例
	dbClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	GateClientA = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);

	//注册反射类
	LoginServer->NetworkObjectClass = UMMORPGLoginServerObject::StaticClass();
	dbClient->NetworkObjectClass = UMMORPGdbClientObject::StaticClass();
	GateClientA->NetworkObjectClass = UMMORPGGateClientObject::StaticClass();

	//初始化服务器
	if (!LoginServer->Init())
	{
		delete LoginServer;
		UE_LOG(LogMMORPGLoginServer, Error, TEXT("LoginServer Init fail."));
		return INDEX_NONE;
	}

	//初始化客户端
	if (!dbClient->Init(11221))
	{
		delete dbClient;
		UE_LOG(LogMMORPGLoginServer, Error, TEXT("dbClient Init fail."));
		return INDEX_NONE;
	}
	if (!GateClientA->Init(11222))
	{
		delete GateClientA;
		UE_LOG(LogMMORPGLoginServer, Error, TEXT("GateClientA Init fail."));
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
		dbClient->Tick(DeltaSeconds);
		GateClientA->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FSimpleNetManage::Destroy(LoginServer);
	FSimpleNetManage::Destroy(dbClient);
	FSimpleNetManage::Destroy(GateClientA);
	FEngineLoop::AppExit();
	return 0;
}
