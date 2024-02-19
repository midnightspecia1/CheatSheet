// Fill out your copyright notice in the Description page of Project Settings.


#include "CheatSheet_GameplayDebuggerCategory.h"

#include "CanvasItem.h"
#include "Interfaces/IPluginManager.h"


namespace
{
	static FAutoConsoleVariable CVarAllowPkgVirtualization(TEXT("Mobs.AllowPkgVirtualization"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarAllowMobs(TEXT("Mobs.Abv.kgVirtualization"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarVirtualization(TEXT("Mobs.asd.AllowPkgVirtualization"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));

}


FCheatSheet_GameplayDebuggerCategory::FCheatSheet_GameplayDebuggerCategory()
{
	bShowOnlyWithDebugActor = false;
	bShowDataPackReplication = true;
	CollectDataInterval = 2.f;
	SetDataPackReplication<FRepData>(&DataPack);

	const FGameplayDebuggerInputHandlerConfig CycleActorReference(TEXT("Next cheat page"), TEXT("Subtract"), FGameplayDebuggerInputModifier::Shift);
	const FGameplayDebuggerInputHandlerConfig CycleNavigationData(TEXT("Previous cheat page"), TEXT("Add"), FGameplayDebuggerInputModifier::Shift);
	
	BindKeyPress(CycleActorReference, this, &FCheatSheet_GameplayDebuggerCategory::DrawNextPage, EGameplayDebuggerInputMode::Replicated);
	BindKeyPress(CycleNavigationData, this, &FCheatSheet_GameplayDebuggerCategory::DrawPrevPage, EGameplayDebuggerInputMode::Replicated);
}

void FCheatSheet_GameplayDebuggerCategory::DrawData(APlayerController* OwnerPC,	FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("Founded console variables: {yellow}"));
	if(DataPack.ConsoleVariableNames.IsEmpty())
	{
		CanvasContext.Printf(TEXT("Console variables were not found, fill CheatSheetConfig.ini !{yellow}"));
	}
	
	for(int32 i = 0; i < DataPack.ConsoleVariableNames.Num() - 1; ++i)
	{
		CanvasContext.Printf(TEXT("%ls {white}%ls"), *DataPack.ConsoleVariableNames[i], *DataPack.ConsoleVariableDescriptions[i]);
	}

	FGameplayDebuggerCategory::DrawData(OwnerPC, CanvasContext);
}

void FCheatSheet_GameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	FString ConfigFilePath = IPluginManager::Get().FindPlugin("CheatSheet")->GetBaseDir() / TEXT("Config") / TEXT("CheatSheetConfig.ini");
	GConfig->GetArray(TEXT("CheatSheetConfig"), TEXT("Cheat"), CheatKeyWords, ConfigFilePath);
	
	const IConsoleManager& ConsoleManager = IConsoleManager::Get();
	auto OnConsoleVariable = [&](const TCHAR *Name, IConsoleObject* CVar)
	{
		DataPack.ConsoleVariableNames.Add(Name);
		DataPack.ConsoleVariableDescriptions.Add(CVar->GetHelp()); 
		UE_LOG(LogTemp, Warning, TEXT("Variable Name: %s"), Name);
	};
	
	for(auto& KeyWord : CheatKeyWords)
	{
		ConsoleManager.ForEachConsoleObjectThatContains(FConsoleObjectVisitor::CreateLambda(OnConsoleVariable), *KeyWord);
	}
}

TSharedRef<FGameplayDebuggerCategory> FCheatSheet_GameplayDebuggerCategory::MakeInstance()
{
	return MakeShareable(new FCheatSheet_GameplayDebuggerCategory());
}

void FCheatSheet_GameplayDebuggerCategory::FRepData::Serialize(FArchive& Ar)
{
	int32 NumNames = ConsoleVariableNames.Num();
	Ar << NumNames;
	if (Ar.IsLoading())
	{
		ConsoleVariableNames.SetNum(NumNames);
	}

	for (int32 Idx = 0; Idx < NumNames; Idx++)
	{
		Ar << ConsoleVariableNames[Idx];
	}
	
	Ar << ConsoleVariableDescriptions;
	int32 NumDescriptions = ConsoleVariableDescriptions.Num();
	Ar << NumDescriptions;
	if (Ar.IsLoading())
	{
		ConsoleVariableDescriptions.SetNum(NumDescriptions);
	}

	for (int32 Idx = 0; Idx < NumDescriptions; Idx++)
	{
		Ar << ConsoleVariableDescriptions[Idx];
	}
}

void FCheatSheet_GameplayDebuggerCategory::DrawNextPage()
{
}

void FCheatSheet_GameplayDebuggerCategory::DrawPrevPage()
{
}

TPair<float, float> FCheatSheet_GameplayDebuggerCategory::EvalLongestString(const TArray<FString>& Strings, const FGameplayDebuggerCanvasContext& CanvasContext)
{
	float MaxX = 0.f;
	float MaxY = 0.f;
	for(auto String : Strings)
	{
		float X = 0.f, Y = 0.f;
		CanvasContext.MeasureString(String, X, Y);
		MaxX = X > MaxX ? X : MaxX;
		MaxY = Y > MaxY ? Y : MaxY;
	}
	return TPair<float, float>(MaxX, MaxY);
}

float FCheatSheet_GameplayDebuggerCategory::EvalStringLength(const FString& String, const FGameplayDebuggerCanvasContext& CanvasContext)
{
	float X = 0.f, Y = 0.f;
	CanvasContext.MeasureString(String, X, Y);
	return X;
}
