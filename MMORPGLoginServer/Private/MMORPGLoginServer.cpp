// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGLoginServer.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMMORPGLoginServer, Log, All);

IMPLEMENT_APPLICATION(MMORPGLoginServer, "MMORPGLoginServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGLoginServer, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
