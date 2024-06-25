#include "ServerObject.h"
#include "Protocols/TestServerProtocols.h"
#include "TestServerALog.h"

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

			//协议回调
			FString HelloResp = TEXT("OK!");
			SIMPLE_PROTOCOLS_SEND(SP_HelloResponses, HelloResp);
			UE_LOG(LogTestServerA, Display, TEXT("AServer send %s"), *HelloResp);
		}
		break;
	}
}

