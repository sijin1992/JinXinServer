#include "MMORPGdbClientObject.h"
#include "Protocol/LoginProtocol.h"
#include "MMORPGLoginServer/Private/ServerList.h"

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
		//dbServer发送回来的登录协议回调
		break;
	}
}

