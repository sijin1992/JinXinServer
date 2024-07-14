#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "MMORPGGateClientObject.generated.h"

UCLASS()
class UMMORPGGateClientObject : public USimpleController
{
	GENERATED_BODY()

public:
	virtual void Init();
	virtual void Tick(float DeltaTime);
	virtual void Close();
	virtual void RecvProtocol(uint32 InProtocol);
};