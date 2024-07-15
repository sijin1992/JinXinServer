#include "MMORPGLoginServerObject.h"
#include "Log/MMORPGLoginServerLog.h"
#include "Protocol/LoginProtocol.h"
#include "MMORPGLoginServer/Private/ServerList.h"

void UMMORPGLoginServerObject::Init()
{
	Super::Init();
}

void UMMORPGLoginServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGLoginServerObject::Close()
{
	Super::Close();
}

void UMMORPGLoginServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_LoginRequests:
		FString AccountString;
		FString PasswordString;
		//接收客户端请求登录的协议
		SIMPLE_PROTOCOLS_RECEIVE(SP_LoginRequests, AccountString, PasswordString);

		//获取本地服务器地址信息
		FSimpleAddrInfo AddrInfo;
		GetAddrInfo(AddrInfo);
		//把本地服务器当作客户端向数据库服务器转发协议
		SIMPLE_CLIENT_SEND(dbClient, SP_LoginRequests, AccountString, PasswordString, AddrInfo);

		UE_LOG(LogMMORPGLoginServer, Display, TEXT("[SP_LoginRequests] AccountString=%s,PasswordString=%s"), *AccountString, *PasswordString);

		break;
	}
}

