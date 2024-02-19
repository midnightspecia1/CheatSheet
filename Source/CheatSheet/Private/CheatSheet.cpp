// Copyright Epic Games, Inc. All Rights Reserved.

#include "CheatSheet.h"

#include "CheatSheet_GameplayDebuggerCategory.h"
#include "GameplayDebugger.h"

#define LOCTEXT_NAMESPACE "FCheatSheetModule"

void FCheatSheetModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory(
		"CheatSheet",
		IGameplayDebugger::FOnGetCategory::CreateStatic(
			&FCheatSheet_GameplayDebuggerCategory::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate, 
		7);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FCheatSheetModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCheatSheetModule, CheatSheet)