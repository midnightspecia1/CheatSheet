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

namespace FCheatSheetNamespace
{
	float RefreshInterval = 5.f;
	int32 MaxCheatPageRows = 20;
}

FCheatSheet_GameplayDebuggerCategory::FCheatSheet_GameplayDebuggerCategory()
{
	bShowOnlyWithDebugActor = false;
	bShowDataPackReplication = true;
	CollectDataInterval = FCheatSheetNamespace::RefreshInterval;
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

	const int32 StartingCheatIndex = FCheatSheetNamespace::MaxCheatPageRows * CurrentCheatPage;
	const int32 EndingCheatIndex = StartingCheatIndex + FCheatSheetNamespace::MaxCheatPageRows;
	const int32 EndingCheatIndexSafe = EndingCheatIndex <= DataPack.ConsoleVariableNames.Num()
	? EndingCheatIndex
	: (DataPack.ConsoleVariableNames.Num() - EndingCheatIndex);

	UE_LOG(LogTemp, Warning, TEXT("StartingCheatIndex: %i, EndingCheatIndexSafe: %i"), StartingCheatIndex, EndingCheatIndexSafe);
	for(int32 i = StartingCheatIndex; i < EndingCheatIndexSafe - 1; ++i)
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

	MaxPagesCount = FMath::FloorToInt32(static_cast<float>(DataPack.ConsoleVariableNames.Num() / FCheatSheetNamespace::MaxCheatPageRows));
	bDataCollected = true;
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
	CurrentCheatPage = (CurrentCheatPage + 1) <= MaxPagesCount ? CurrentCheatPage++ : 0;
	UE_LOG(LogTemp, Warning, TEXT("NextPage Draw, CurrentCheatPage: %i"), CurrentCheatPage);
}

void FCheatSheet_GameplayDebuggerCategory::DrawPrevPage()
{
	CurrentCheatPage = (CurrentCheatPage - 1) >= 0 ? CurrentCheatPage-- : MaxPagesCount;
	UE_LOG(LogTemp, Warning, TEXT("PrevPage Draw, CurrentCheatPage: %i"), CurrentCheatPage);
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
