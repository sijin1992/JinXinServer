// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGCenterServer.h"
#include "Log/MMORPGCenterServerLog.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "SimpleNetManage.h"
#include "ServerList.h"
#include "CenterServer/MMORPGCenterServerObject.h"
#include "CenterServer/MMORPGdbClientObject.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(MMORPGCenterServer, "MMORPGCenterServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGCenterServer, Display, TEXT("MMORPGCenterServer Beginning"));

	//初始化插件
	FSimpleNetGlobalInfo::Get()->Init();

	//创建服务器实例
	CenterServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);
	//创建db客户端
	dbClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_TCP);

	//注册反射类
	CenterServer->NetworkObjectClass = UMMORPGCenterServerObject::StaticClass();
	dbClient->NetworkObjectClass = UMMORPGdbClientObject::StaticClass();

	//初始化服务器
	if (!CenterServer->Init())
	{
		delete CenterServer;
		UE_LOG(LogMMORPGCenterServer, Error, TEXT("CenterServer Init fail."));
		return INDEX_NONE;
	}

	//初始化客户端
	if (!dbClient->Init(11221))
	{
		delete dbClient;
		UE_LOG(LogMMORPGCenterServer, Error, TEXT("dbClient Init fail."));
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
		CenterServer->Tick(DeltaSeconds);
		dbClient->Tick(DeltaSeconds);

		LastTime = Now;
	}

	FSimpleNetManage::Destroy(CenterServer);
	FSimpleNetManage::Destroy(dbClient);

	FEngineLoop::AppExit();
	return 0;
}
