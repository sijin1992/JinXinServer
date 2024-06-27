#include "ClientObjectB.h"
#include "Protocols/TestServerProtocols.h"
#include "TestServerALog.h"
#include "TestServerList.h"

void UClientObjectB::Init()
{
	Super::Init();
}

void UClientObjectB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UClientObjectB::Close()
{
	Super::Close();
}

void UClientObjectB::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);
	Super::RecvProtocol(InProtocol);
	//接收服务器回调协议
	switch (InProtocol)
	{
	case SP_HelloResponses:
		FString Hello2String;
		FSimpleAddrInfo AddrInfo;
		SIMPLE_PROTOCOLS_RECEIVE(SP_HelloResponses, Hello2String, AddrInfo);

		SIMPLE_SERVER_SEND(LocalServer, SP_HelloResponses, AddrInfo, Hello2String)

		UE_LOG(LogTestServerA, Display, TEXT("AServer send %s"), *Hello2String);

		break;
	}
}

