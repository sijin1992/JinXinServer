// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGGateServer.h"
#include "Log/MMORPGGateServerLog.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(MMORPGGateServer, "MMORPGGateServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGGateServer, Display, TEXT("MMORPGGateServer Beginning"));
	FEngineLoop::AppExit();
	return 0;
}
