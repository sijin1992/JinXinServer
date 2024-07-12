// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGGateServer.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMMORPGGateServer, Log, All);

IMPLEMENT_APPLICATION(MMORPGGateServer, "MMORPGGateServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGGateServer, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
