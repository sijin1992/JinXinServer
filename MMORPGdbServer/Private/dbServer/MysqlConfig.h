#pragma once
#include "MMORPGdbServerType.h"

//密码验证状态
enum EPasswordVerification
{
	VERIFICATION_SUCCESS = 0,	//成功
	VERIFICATION_FAIL,			//失败
};

//只是框架
class FSimpleMysqlConfig
{
public:
	static FSimpleMysqlConfig* Get();
	static void Destroy();

	void Init(const FString& InPath = FPaths::ProjectDir()/TEXT("MysqlConfig.ini"));
	const FMysqlConfig& GetInfo() const;

private:
	static FSimpleMysqlConfig* Global;//单例
	FMysqlConfig ConfigInfo;
};