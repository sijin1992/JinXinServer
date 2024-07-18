#include "MMORPGdbClientObject.h"
#include "Log/MMORPGGateServerLog.h"
#include "Protocol/HallProtocol.h"
#include "ServerList.h"

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
	case SP_CharacterAppearanceResponses:
		//收到dbServer返回的角色信息回调
		FString String;//dbServer发送回来的角色Json数据
		FSimpleAddrInfo AddrInfo;
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_CharacterAppearanceResponses, AddrInfo, String);

		//向本地服务器转发协议,
		//SIMPLE_SERVER_SEND：客户端受到消息后，由服务器发送到另外一端
		//SIMPLE_CLIENT_SEND: 服务器通过客户端发送到对方的服务器
		SIMPLE_SERVER_SEND(GateServer, SP_CharacterAppearanceResponses, AddrInfo, String);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_CharacterAppearanceResponses]"));
		break;
	}
}

