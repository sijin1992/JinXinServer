// Copyright Epic Games, Inc. All Rights Reserved.


#include "MMORPGCenterServer.h"

#include "RequiredProgramMainCPPInclude.h"

DEFINE_LOG_CATEGORY_STATIC(LogMMORPGCenterServer, Log, All);

IMPLEMENT_APPLICATION(MMORPGCenterServer, "MMORPGCenterServer");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMMORPGCenterServer, Display, TEXT("Hello World"));
	FEngineLoop::AppExit();
	return 0;
}
