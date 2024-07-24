#include "MMORPGCenterServerObject.h"
#include "Log/MMORPGCenterServerLog.h"
#include "Protocol/HallProtocol.h"
#include "Protocol/ServerProtocol.h"
#include "../ServerList.h"

void UMMORPGCenterServerObject::Init()
{
	Super::Init();
	//预分配注册玩家信息列表
	for (int32 i = 0; i < 2000; i++)
	{
		PlayerRegistInfos.Add(i, FMMORPGPlayerRegistInfo());
	}
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
		//收到GateServer发过来的登录专属服务器请求3
		int32 InUserID = INDEX_NONE;
		int32 SlotID = INDEX_NONE;
		FSimpleAddrInfo GateAddrInfo;//网关地址
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginToDSServerRequests, InUserID, SlotID, GateAddrInfo);
		UE_LOG(LogMMORPGCenterServer, Display, TEXT("[SP_LoginToDSServerRequests]"));

		if (InUserID != INDEX_NONE && SlotID != INDEX_NONE)
		{
			FSimpleAddrInfo CenterAddrInfo;//网关地址
			GetRemoteAddrInfo(CenterAddrInfo);//GetRemoteAddrInfo和GetAddrInfo一样，都是获取远端连接的地址(Channel的地址)
			//向db服务器请求用户注册信息4
			SIMPLE_CLIENT_SEND(dbClient, SP_PlayerRegistInfoRequests, InUserID, SlotID, GateAddrInfo, CenterAddrInfo);
			UE_LOG(LogMMORPGCenterServer, Display, TEXT("[SP_PlayerRegistInfoRequests]"));
			//将用户角色注册到列表里

			//通过DSServer获取存档
			//向本地服务器转发协议
			//FSimpleAddr Addr;
			//SIMPLE_PROTOCOLS_SEND(SP_LoginToDSServerRequests, GateAddrInfo, Addr);
		}
		break;
	}
	}
}

void UMMORPGCenterServerObject::AddRegistInfo(const FMMORPGPlayerRegistInfo& InRegistInfo)
{
	for (auto& Temp : PlayerRegistInfos)
	{
		if (!Temp.Value.IsValid())
		{
			Temp.Value = InRegistInfo;
			break;
		}
	}
}
