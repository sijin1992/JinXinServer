#pragma once

#include "CoreMinimal.h"
#include "UObject/SimpleController.h"
#include "Core/SimpleMysqlLinkType.h"
#include "MMORPGServerObject.generated.h"

class USimpleMysqlObject;
UCLASS()
class UMMORPGServerObject : public USimpleController
{
	GENERATED_BODY()

public:
	virtual void Init();//当有新的链接连接当前服务器时触发
	virtual void Tick(float DeltaTime);
	virtual void Close();
	virtual void RecvProtocol(uint32 InProtocol);//当对方向当前服务器发送协议时触发

public:
	/// <summary>
	/// 提交
	/// </summary>
	/// <param name="InSQL">SQL语句</param>
	/// <returns></returns>
	bool Post(const FString& InSQL);
	/// <summary>
	/// 获取
	/// </summary>
	/// <param name="InSQL">SQL语句</param>
	/// <param name="Result">返回结果</param>
	/// <returns></returns>
	bool Get(const FString& InSQL, TArray<FSimpleMysqlResult>& Results);

protected:
	USimpleMysqlObject* MysqlObjectRead;//负责读取数据库
	USimpleMysqlObject* MysqlObjectWrite;//负责向数据库写入数据
};
