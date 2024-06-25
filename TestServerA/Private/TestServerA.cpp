// Copyright Epic Games, Inc. All Rights Reserved.


#include "TestServerA.h"

#include "RequiredProgramMainCPPInclude.h"
#include "SimpleNetManage.h"
#include "Global/SimpleNetGlobalInfo.h"
#include "TestServerList.h"
#include "Server/ServerObject.h"
#include "Server/ClientObjectB.h"
#include "Server/ClientObjectC.h"

DEFINE_LOG_CATEGORY_STATIC(LogTestServerA, Log, All);

IMPLEMENT_APPLICATION(TestServerA, "TestServerA");

//相当于Main函数入口
INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);

	//1、初始化配置表
	FSimpleNetGlobalInfo::Get()->Init();//初始化通道

	//2、创建实例
	//UDP监听服务器
	LocalServer = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_LISTEN, ESimpleSocketType::SIMPLESOCKETTYPE_UDP);
	//UDP连接客户端B
	BClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_UDP);
	//UDP连接客户端C
	CClient = FSimpleNetManage::CreateManage(ESimpleNetLinkState::LINKSTATE_CONNET, ESimpleSocketType::SIMPLESOCKETTYPE_UDP);

	UE_LOG(LogTestServerA, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
