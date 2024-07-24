#include "MMORPGCenterClientObject.h"
#include "Log/MMORPGGateServerLog.h"
#include "Protocol/HallProtocol.h"
#include "Protocol/GameProtocol.h"
#include "../ServerList.h"

void UMMORPGCenterClientObject::Init()
{
	Super::Init();
}

void UMMORPGCenterClientObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGCenterClientObject::Close()
{
	Super::Close();
}

void UMMORPGCenterClientObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_LoginToDSServerResponses:
	{
		//收到CenterServer发过来的登录专属服务器回调9
		FString String;
		FSimpleAddrInfo GateAddrInfo;//网关服务器地址
		FSimpleAddr DSAddr;//专属服务器地址
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginToDSServerResponses, GateAddrInfo, DSAddr);

		//向本地服务器转发协议
		SIMPLE_SERVER_SEND(GateServer, SP_LoginToDSServerResponses, GateAddrInfo, DSAddr);

		double NowTime = FPlatformTime::Seconds();
		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_LoginToDSServerResponses 9] NowTime=%d"), NowTime);

		break;
	}
	}
}
