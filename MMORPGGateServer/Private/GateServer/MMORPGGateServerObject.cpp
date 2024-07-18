#include "MMORPGGateServerObject.h"
#include "Log/MMORPGGateServerLog.h"
#include "Protocol/ServerProtocol.h"
#include "MMORPGType.h"
#include "Protocol/HallProtocol.h"
#include "ServerList.h"

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
	{
		//收到网关状态请求协议
		//准备数据
		FMMORPGGateStatus Status;
		//GetAddrInfo：获取连接到服务器的对象的地址
		//GetServerAddrInfo:获取当前服务器的地址
		//获取网关服务器地址
		GetServerAddrInfo(Status.GateServerAddrInfo);
		Status.GateConnetionNum = GetManage()->GetConnetionNum();
		//发送回调
		SIMPLE_PROTOCOLS_SEND(SP_GateStatusResponses, Status);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("SP_GateStatusResponses"));
		break;
	}
	case SP_CharacterAppearanceRequests:
	{
		//收到获取角色信息请求
		int32 InUserID = INDEX_NONE;
		SIMPLE_PROTOCOLS_RECEIVE(SP_CharacterAppearanceRequests, InUserID);

		//获取本地服务器地址信息
		FSimpleAddrInfo AddrInfo;
		GetAddrInfo(AddrInfo);
		//把本地服务器当作客户端向数据库服务器转发协议
		SIMPLE_CLIENT_SEND(dbClient, SP_CharacterAppearanceRequests, InUserID, AddrInfo);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_CharacterAppearanceRequests] InUserID=%i"), InUserID);

		break;
	}
	}
}

