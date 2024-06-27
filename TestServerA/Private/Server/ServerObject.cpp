#include "ServerObject.h"
#include "Protocols/TestServerProtocols.h"
#include "TestServerALog.h"
#include "TestServerList.h"

void UServerObject::Init()
{
	Super::Init();
}

void UServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UServerObject::Close()
{
	Super::Close();
}

void UServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);
	//接收其他服务器里的客户端发送过来的协议
	switch (InProtocol)
	{
		case SP_HelloRequests:
			FString HelloString;
			int32 vvv = INDEX_NONE;
			SIMPLE_PROTOCOLS_RECEIVE(SP_HelloRequests, HelloString, vvv);

			if (!HelloString.IsEmpty())
			{
				UE_LOG(LogTestServerA, Display, TEXT("AServer Recv %s %i"), *HelloString, vvv);

			
				FString HelloResp = TEXT("OK!");
				//协议回调(当接收到C服务器中的A客户端向A服务器发送的协议时，通过A服务器直接发送协议回复给C服务器中的A客户端)
				//SIMPLE_PROTOCOLS_SEND(SP_HelloResponses, HelloResp);
				////协议转发,一定要记录地址(当A服务器接收到C服务器中的A客户端发送的协议时，将这个协议通过A服务器中的B客户端转发给B服务器)
				//FSimpleAddrInfo AddrInfo;//当前服务器A自身的地址
				//GetAddrInfo(AddrInfo);
				//SIMPLE_CLIENT_SEND(BClient, SP_HelloRequests, HelloString, AddrInfo)
				UE_LOG(LogTestServerA, Display, TEXT("AServer send %s"), *HelloResp);
			}
			break;
	}
}

