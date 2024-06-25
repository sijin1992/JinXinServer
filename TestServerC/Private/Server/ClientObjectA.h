#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "ClientObjectA.generated.h"

UCLASS()
class UClientObjectA : public USimpleController
{
	GENERATED_BODY()
public:
	UClientObjectA();

protected:
	float Time;

	//主线程运行
	virtual void Init();
	//主线程运行
	virtual void Tick(float DeltaTime);
	//主线程运行
	virtual void Close();
	//单线程服务器在主线程运行，高并发服务器在其他线程运行
	virtual void RecvProtocol(uint32 InProtocol);
};