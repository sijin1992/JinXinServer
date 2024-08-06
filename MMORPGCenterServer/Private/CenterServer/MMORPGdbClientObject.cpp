#include "MMORPGdbClientObject.h"
#include "Log/MMORPGCenterServerLog.h"
#include "Protocol/HallProtocol.h"
#include "Protocol/ServerProtocol.h"
#include "../ServerList.h"
#include "MMORPGCenterServerObject.h"

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
	case SP_PlayerRegistInfoResponses:
	{
		//收到dbServer发过来的玩家注册信息回调7
		FString UserInfoJson;
		FString SlotInfoJson;
		FSimpleAddrInfo GateAddrInfo;//网关地址
		FSimpleAddrInfo CenterAddrInfo;//中心服务器地址
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_PlayerRegistInfoResponses, UserInfoJson, SlotInfoJson, GateAddrInfo, CenterAddrInfo);

		UE_LOG(LogMMORPGCenterServer, Display, TEXT("[SP_PlayerRegistInfoResponses]"));

		if (UserInfoJson != TEXT("[]") && SlotInfoJson != TEXT("[]"))
		{
			//注册成功
			FMMORPGPlayerRegistInfo InRegistInfo;
			NetDataAnalysis::StringToFCharacterAppearacnce(SlotInfoJson, InRegistInfo.CAInfo);
			NetDataAnalysis::StringToUserData(UserInfoJson, InRegistInfo.UserInfo);

			UMMORPGCenterServerObject::AddRegistInfo(InRegistInfo);
			//准备DS服务器地址信息,通过中心服务器向网关服务器转发登录到DS服务器回调8
			FSimpleAddr DsAddr = FSimpleNetManage::GetSimpleAddr(TEXT("127.0.0.1"),7777);//DS服务器地址,7777是UE5的端口
			SIMPLE_SERVER_SEND(CenterServer, SP_LoginToDSServerResponses, CenterAddrInfo, GateAddrInfo, DsAddr);

			double NowTime = FPlatformTime::Seconds();
			UE_LOG(LogMMORPGCenterServer, Display, TEXT("[SP_LoginToDSServerResponses 8] NowTime=%d"), NowTime);
		}
		break;
	}
	}
}