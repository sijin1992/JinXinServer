// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGdbServer.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMMORPGdbServer, Log, All);

IMPLEMENT_APPLICATION(MMORPGdbServer, "MMORPGdbServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGdbServer, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
