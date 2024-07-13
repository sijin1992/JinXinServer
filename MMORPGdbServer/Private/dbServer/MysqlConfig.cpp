#include "MysqlConfig.h"
#include "Misc/FileHelper.h"

FSimpleMysqlConfig* FSimpleMysqlConfig::Global = nullptr;

FSimpleMysqlConfig* FSimpleMysqlConfig::Get()
{
	if (!Global)
	{
		Global = new FSimpleMysqlConfig();
	}
	return Global;
}

void FSimpleMysqlConfig::Destroy()
{
	if (Global)
	{
		delete Global;
		Global = nullptr;
	}
}

void FSimpleMysqlConfig::Init(const FString& InPath /*= FPaths::ProjectDir()/TEXT("MysqlConfig.ini")*/)
{
	TArray<FString> Content;
	FFileHelper::LoadFileToStringArray(Content, *InPath);
	if (Content.Num())
	{
		//如果存在，就解析
		auto Lamabda = [&](TMap<FString, FString>& OutContent)
			{
				for (auto& Temp : Content)
				{
					if (Temp.Contains("[") && Temp.Contains("]"))//解析Head
					{
						Temp.RemoveFromEnd("]");
						Temp.RemoveFromStart("[");

						OutContent.Add("ConfigHead", Temp);
					}
					else//解析Body
					{
						FString R, L;
						Temp.Split(TEXT("="), &L, &R);

						OutContent.Add(L, R);
					}
				}
			};

		TMap<FString, FString> InConfigInfo;
		Lamabda(InConfigInfo);

		ConfigInfo.User = InConfigInfo["User"];
		ConfigInfo.Host = InConfigInfo["Host"];
		ConfigInfo.Pawd = InConfigInfo["Pawd"];
		ConfigInfo.DB = InConfigInfo["DB"];
		ConfigInfo.Port = FCString::Atoi(*(InConfigInfo["Port"]));
	}
	else
	{
		//手动创建
		Content.Add(TEXT("[SimpleMysqlConfig]"));
		Content.Add(FString::Printf(TEXT("User=%s"), *ConfigInfo.User));
		Content.Add(FString::Printf(TEXT("Host=%s"), *ConfigInfo.Host));
		Content.Add(FString::Printf(TEXT("Pawd=%s"), *ConfigInfo.Pawd));
		Content.Add(FString::Printf(TEXT("DB=%s"), *ConfigInfo.DB));
		Content.Add(FString::Printf(TEXT("Port=%i"), ConfigInfo.Port));

		FFileHelper::SaveStringArrayToFile(Content, *InPath);
	}
}

const FMysqlConfig& FSimpleMysqlConfig::GetInfo() const
{
	return ConfigInfo;
}
