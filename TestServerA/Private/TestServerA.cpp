// Copyright Epic Games, Inc. All Rights Reserved.


#include "TestServerA.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogTestServerA, Log, All);

IMPLEMENT_APPLICATION(TestServerA, "TestServerA");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogTestServerA, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
