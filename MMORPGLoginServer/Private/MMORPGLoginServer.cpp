// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGLoginServer.h"
#include "Log/MMORPGLoginServerLog.h"
#include "RequiredProgramMainCPPInclude.h"

IMPLEMENT_APPLICATION(MMORPGLoginServer, "MMORPGLoginServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGLoginServer, Display, TEXT("MMORPGLoginServer Beginning"));
	FEngineLoop::AppExit();
	return 0;
}
