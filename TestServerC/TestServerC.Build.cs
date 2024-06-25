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
		//新增
		PrivateDependencyModuleNames.Add("ApplicationCore");
		PrivateDependencyModuleNames.Add("SimpleNetChannel");//SNC
		PrivateDependencyModuleNames.Add("CoreUObject");//要用到反射
	}
}
