#include "MMORPGGateServerObject.h"
#include "Log/MMORPGGateServerLog.h"
#include "Protocol/ServerProtocol.h"
#include "MMORPGType.h"

void UMMORPGGateServerObject::Init()
{
	Super::Init();
}

void UMMORPGGateServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGGateServerObject::Close()
{
	Super::Close();
}

void UMMORPGGateServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_GateStatusRequests:
		//收到网关状态请求协议
		//准备数据
		FMMORPGGateStatus Status;
		GetAddrInfo(Status.GateServerAddrInfo);
		Status.GateConnetionNum = GetManage()->GetConnetionNum();
		//发送回调
		SIMPLE_PROTOCOLS_SEND(SP_GateStatusResponses, Status);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("SP_GateStatusResponses"));
		break;
	}
}

