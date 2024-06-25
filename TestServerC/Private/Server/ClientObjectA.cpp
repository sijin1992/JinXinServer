#include "ClientObjectA.h"
#include "Protocols/TestServerProtocols.h"
#include "TestServerCLog.h"

UClientObjectA::UClientObjectA()
{
	Time = 0.0f;
}

void UClientObjectA::Init()
{
	Super::Init();
}

void UClientObjectA::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsMainConnetion())//判断是否是主连接
	{
		Time += DeltaTime;
		if (Time >= 5.0f)
		{
			Time = 0.0f;
			//C服务器中的A客户端向A服务器发送协议
			FString HelloString = TEXT("Hello Server A!");//参数不能加const : const FString HelloString = TEXT("Hello Server A!");
			int32 vvv = 100;
			SIMPLE_PROTOCOLS_SEND(SP_HelloRequests, HelloString, vvv);//协议名前要加SP_

			UE_LOG(LogTestServerC, Display, TEXT("CServer_AClient send %s %i"), *HelloString, vvv);
		}
	}
}

void UClientObjectA::Close()
{
	Super::Close();
}

void UClientObjectA::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);
	//接收服务器回调协议
	switch (InProtocol)
	{
	case SP_HelloResponses:
		FString HelloResp = TEXT("OK!");
		SIMPLE_PROTOCOLS_RECEIVE(SP_HelloResponses, HelloResp);

		if (!HelloResp.IsEmpty())
		{
			UE_LOG(LogTestServerC, Display, TEXT("CServer_AClient Recv %s"), *HelloResp);
		}
		break;
	}
}

