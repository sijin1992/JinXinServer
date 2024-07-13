#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "MMORPGServerObject.generated.h"

UCLASS()
class UMMORPGServerObject : public USimpleController
{
	GENERATED_BODY()

public:
	virtual void Init();//当有新的链接连接当前服务器时触发
	virtual void Tick(float DeltaTime);
	virtual void Close();
	virtual void RecvProtocol(uint32 InProtocol);//当对方向当前服务器发送协议时触发
};
