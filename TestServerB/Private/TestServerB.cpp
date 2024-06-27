// Copyright Epic Games, Inc. All Rights Reserved.


#include "TestServerB.h"

#include "RequiredProgramMainCPPInclude.h"
#include "SimpleNetManage.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "TestServerList.h"
#include "Server/ServerObject.h"
#include "Server/ClientObjectA.h"
#include "Server/ClientObjectC.h"
#include "TestServerBLog.h"

IMPLEMENT_APPLICATION(TestServerB, "TestServerB");

//相当于Main函数入口
INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);

	//1、初始化配置表
	FSimpleNetGlobalInfo::Get()->Init();//初始化通道

	//2、创建实例
	//UDP监听服务器
	LocalServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_UDP,true);
	//UDP连接客户端A
	AClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_UDP,true);
	//UDP连接客户端C
	CClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_UDP,true);

	//3、反射类注入：利用反射绑定类
	LocalServer->NetworkObjectClass = UServerObject::StaticClass();
	AClient->NetworkObjectClass = UClientObjectA::StaticClass();
	CClient->NetworkObjectClass = UClientObjectC::StaticClass();

	//4、服务器和客户端的初始化
	//初始化服务器
	if (!LocalServer->Init())
	{
		delete LocalServer;
		UE_LOG(LogTestServerB, Error, TEXT("LocalServer Init fail."));
		return -1;
	}
	//初始化A客户端，12345是随便写的端口
	if (!AClient->Init(10224))
	{
		delete AClient;
		UE_LOG(LogTestServerB, Error, TEXT("AClient Init fail."));
		return -1;
	}
	//初始化C客户端
	if (!CClient->Init(10228))
	{
		delete CClient;
		UE_LOG(LogTestServerB, Error, TEXT("CClient Init fail."));
		return -1;
	}

	//5、检测
	double LastTime = FPlatformTime::Seconds();
	//判断引擎是否要退出
	while (!IsEngineExitRequested())
	{
		FPlatformProcess::Sleep(0.03f);//让引擎默认休眠3秒

		//获取时间间隔
		double Now = FPlatformTime::Seconds();
		float DeltaSeconds = Now - LastTime;

		//执行Tick
		LocalServer->Tick(DeltaSeconds);
		AClient->Tick(DeltaSeconds);
		CClient->Tick(DeltaSeconds);

		//执行UE本身的线程
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(FApp::GetDeltaTime());

		LastTime = Now;
	}

	//6、销毁
	FSimpleNetManage::Destroy(LocalServer);
	FSimpleNetManage::Destroy(AClient);
	FSimpleNetManage::Destroy(CClient);

	UE_LOG(LogTestServerB, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
