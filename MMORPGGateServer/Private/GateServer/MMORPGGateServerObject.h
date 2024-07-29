#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "MMORPGGateServerObject.generated.h"

UCLASS()
class UMMORPGGateServerObject : public USimpleController
{
	GENERATED_BODY()

public:
	UMMORPGGateServerObject();

	virtual void Init();
	virtual void Tick(float DeltaTime);
	virtual void Close();
	virtual void RecvProtocol(uint32 InProtocol);

private:
	int32 UserID;
};