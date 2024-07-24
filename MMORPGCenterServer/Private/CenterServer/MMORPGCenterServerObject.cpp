#include "MMORPGCenterServerObject.h"
#include "Log/MMORPGCenterServerLog.h"
#include "Protocol/HallProtocol.h"
#include "../ServerList.h"

void UMMORPGCenterServerObject::Init()
{
	Super::Init();
}

void UMMORPGCenterServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGCenterServerObject::Close()
{
	Super::Close();
}

void UMMORPGCenterServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_LoginToDSServerRequests:
	{
		//收到GateServer发过来的登录专属服务器请求
		int32 InUserID = INDEX_NONE;
		int32 SlotID = INDEX_NONE;
		FSimpleAddrInfo AddrInfo;
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginToDSServerRequests, InUserID, SlotID, AddrInfo);

		if (InUserID != INDEX_NONE && SlotID != INDEX_NONE)
		{
			//将用户角色注册到列表里
			//通过DSServer获取存档
			//向本地服务器转发协议
			FSimpleAddr Addr;
			SIMPLE_PROTOCOLS_SEND(SP_LoginToDSServerResponses, AddrInfo, Addr);
		}

		UE_LOG(LogMMORPGCenterServer, Display, TEXT("[SP_LoginToDSServerResponses]"));

		break;
	}
	}
}
