#include "MMORPGdbClientObject.h"
#include "Log/MMORPGLoginServerLog.h"
#include "Protocol/LoginProtocol.h"
#include "MMORPGLoginServer/Private/ServerList.h"
#include "MMORPGType.h"
#include "MMORPGGateClientObject.h"

void UMMORPGdbClientObject::Init()
{
	Super::Init();
}

void UMMORPGdbClientObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGdbClientObject::Close()
{
	Super::Close();
}

void UMMORPGdbClientObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);
	switch (InProtocol)
	{
	case SP_LoginResponses:
		FString String;//dbServer发送回来的用户Json数据
		//dbServer发送回来的登录协议回调
		ELoginType Type = ELoginType::LOGIN_DB_SERVER_ERROR;
		FSimpleAddrInfo AddrInfo;
		//接收登录数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginResponses, AddrInfo, Type, String);

		//负载均衡：在收到网关状态回调后，比较网关数据,把网关连接人数最少的发送给客户端
		FMMORPGGateStatus GateStatus;
		if (UMMORPGGateClientObject* InGateClient = Cast<UMMORPGGateClientObject>(GateClientA->GetController()))
		{
			if (InGateClient->GetGateStatus().GateConnetionNum <= 2000)
			{
				GateStatus = InGateClient->GetGateStatus();
			}
		}

		//向本地服务器转发协议,
		//SIMPLE_SERVER_SEND：客户端受到消息后，由服务器发送到另外一端
		//SIMPLE_CLIENT_SEND: 服务器通过客户端发送到对方的服务器
		SIMPLE_SERVER_SEND(LoginServer, SP_LoginResponses, AddrInfo, Type, String, GateStatus);

		UE_LOG(LogMMORPGLoginServer, Display, TEXT("[SP_LoginResponses]"));

		break;
	}
}

