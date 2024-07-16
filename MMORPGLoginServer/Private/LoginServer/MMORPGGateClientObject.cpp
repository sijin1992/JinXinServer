#include "MMORPGGateClientObject.h"
#include "Log/MMORPGLoginServerLog.h"
#include "Protocol/ServerProtocol.h"

UMMORPGGateClientObject::UMMORPGGateClientObject()
	:Time(0)
{

}

void UMMORPGGateClientObject::Init()
{
	Super::Init();
}

void UMMORPGGateClientObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Time += DeltaTime;
	if (Time >= 1.0f)
	{
		Time = 0;

		SIMPLE_PROTOCOLS_SEND(SP_GateStatusRequests);//发送网关状态请求协议来更新网关状态
	}
}

void UMMORPGGateClientObject::Close()
{
	Super::Close();
}

void UMMORPGGateClientObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_GateStatusResponses:
		//GateServer发送回来的网关状态请求回调
		//接收数据,更新网关状态
		SIMPLE_PROTOCOLS_RECEIVE(SP_GateStatusResponses, GateStatus);
		
		UE_LOG(LogMMORPGLoginServer, Display, TEXT("[SP_GateStatusResponses]"));

		break;
	}
}

FMMORPGGateStatus& UMMORPGGateClientObject::GetGateStatus()
{
	return GateStatus;
}

