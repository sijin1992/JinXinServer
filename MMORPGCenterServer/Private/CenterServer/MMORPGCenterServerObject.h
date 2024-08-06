#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "MMORPGType.h"
#include "MMORPGCenterServerObject.generated.h"

UCLASS()
class UMMORPGCenterServerObject : public USimpleController
{
	GENERATED_BODY()

public:
	virtual void Init();
	virtual void Tick(float DeltaTime);
	virtual void Close();
	virtual void RecvProtocol(uint32 InProtocol);

	static void AddRegistInfo(const FMMORPGPlayerRegistInfo& InRegistInfo);
	static bool RemoveRegistInfo(const int32 InUserID);
private:
	static TMap<int32, FMMORPGPlayerRegistInfo> PlayerRegistInfos;//注册玩家信息列表
};