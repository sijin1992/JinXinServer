#include "MMORPGdbClientObject.h"
#include "Log/MMORPGCenterServerLog.h"

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

	//switch (InProtocol)
}