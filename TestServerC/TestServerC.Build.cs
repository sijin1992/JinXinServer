// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestServerC : ModuleRules
{
	public TestServerC(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/Launch/Public");

		PrivateIncludePaths.Add("Runtime/Launch/Private");		// For LaunchEngineLoop.cpp include

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");
		//新增插件
		//系统模块
		PrivateDependencyModuleNames.Add("ApplicationCore");
		PrivateDependencyModuleNames.Add("CoreUObject");//要用到反射
		//自定义插件
		PrivateDependencyModuleNames.Add("SimpleNetChannel");//SNC插件
		PrivateDependencyModuleNames.Add("TestServerCommon");//协议插件
	}
}
