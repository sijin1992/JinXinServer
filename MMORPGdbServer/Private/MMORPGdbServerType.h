#pragma once
#include "CoreMinimal.h"
#include "Core/SimpleMysqlLinkType.h"

//数据库配置
struct FMysqlConfig
{
	FMysqlConfig()
		:User("root"),
		Host("127.0.0.1"),
		Pawd("root"),
		DB("Hello"),
		Port(3306)
	{
		ClientFlags.Add(ESimpleClientFlags::Client_Multi_Statements);
		ClientFlags.Add(ESimpleClientFlags::Client_Multi_Results);
	}

	FString User;//用户名
	FString Host;//主机地址
	FString Pawd;//密码
	FString DB;//数据库
	int32 Port;//端口
	TArray<ESimpleClientFlags> ClientFlags;//行为,支持多条语句
};