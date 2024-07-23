#include "MMORPGCenterServerObject.h"
#include "Log/MMORPGCenterServerLog.h"

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

	//switch (InProtocol)
}
