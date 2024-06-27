#include "ServerObject.h"
#include "Protocols/TestServerProtocols.h"
#include "TestServerBLog.h"

void UServerObject::Init()
{
	Super::Init();
}

void UServerObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UServerObject::Close()
{
	Super::Close();
}

void UServerObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
		case SP_HelloRequests:
			FString HelloString;
			FSimpleAddrInfo AddrInfo;
			SIMPLE_PROTOCOLS_RECEIVE(SP_HelloRequests, HelloString, AddrInfo);

			if (!HelloString.IsEmpty())
			{
				UE_LOG(LogTestServerB, Display, TEXT("BServer Recv %s"), *HelloString);
			}

			break;
	}
}

