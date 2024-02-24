// Fill out your copyright notice in the Description page of Project Settings.


#include "CheatSheet_GameplayDebuggerCategory.h"

#include "CanvasItem.h"
#include "Interfaces/IPluginManager.h"


namespace
{
	static FAutoConsoleVariable CVarAllowPkgVirtualization(TEXT("Mobs.AllowPkgVirtualization"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarAllowMobs(TEXT("Mobs.Abv.kgVirtual"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarVirtualization(TEXT("Mobs.asd.AllowPkgVirtualization"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarAllowPkg(TEXT("Mobs.BBBB"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarAllow(TEXT("Mobs.Abv.AAAAAAAAA"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
	static FAutoConsoleVariable CVarA(TEXT("Mobs.asd.b"),true,TEXT("When true submitting packages in the editor will no longer trigger the virtualization process"));
}

namespace FCheatSheetNamespace
{
	float RefreshInterval = 5.f;
	int32 MaxCheatPageRows = 20;
	FName NextPageKey = TEXT("Add");
	FName PreviousPageKey = TEXT("Subtract");
}

FCheatSheet_GameplayDebuggerCategory::FCheatSheet_GameplayDebuggerCategory()
{
	bShowOnlyWithDebugActor = false;
	bShowDataPackReplication = true;
	CollectDataInterval = FCheatSheetNamespace::RefreshInterval;
	SetDataPackReplication<FRepData>(&DataPack);

	const FGameplayDebuggerInputHandlerConfig NextPageHandler(TEXT("Next cheat page"), FCheatSheetNamespace::NextPageKey, FGameplayDebuggerInputModifier::Shift);
	const FGameplayDebuggerInputHandlerConfig PrevPageHandler(TEXT("Previous cheat page"), FCheatSheetNamespace::PreviousPageKey, FGameplayDebuggerInputModifier::Shift);
	
	BindKeyPress(NextPageHandler, this, &FCheatSheet_GameplayDebuggerCategory::DrawNextPage, EGameplayDebuggerInputMode::Replicated);
	BindKeyPress(PrevPageHandler, this, &FCheatSheet_GameplayDebuggerCategory::DrawPrevPage, EGameplayDebuggerInputMode::Replicated);
}

void FCheatSheet_GameplayDebuggerCategory::DrawData(APlayerController* OwnerPC,	FGameplayDebuggerCanvasContext& CanvasContext)
{
	FVector2D ViewPortSize;
	GEngine->GameViewport->GetViewportSize( /*out*/ViewPortSize);

	float MaxCheatWidth = 0.0f;
	// Find longest cheat name
	for(int32 i = 0; DataPack.ConsoleVariableNames.Num() > i; i++)
	{
		float StrCheatWidth = 0.0f, StrCheatHeight = 0.0f;
		CanvasContext.MeasureString(DataPack.ConsoleVariableNames[i], StrCheatWidth, StrCheatHeight);
		if(StrCheatWidth > MaxCheatWidth)
		{
			MaxCheatWidth = StrCheatWidth;
		}
	}

	CanvasContext.Printf(TEXT("Use Shift + %s/%s to cycle pages {yellow}"), *FCheatSheetNamespace::NextPageKey.ToString(), *FCheatSheetNamespace::PreviousPageKey.ToString());
	CanvasContext.Printf(TEXT("Founded console variables: {yellow}"));
	if(DataPack.ConsoleVariableNames.IsEmpty())
	{
		CanvasContext.Printf(TEXT("Console variables were not found, fill CheatSheetConfig.ini !{yellow}"));
	}

	const int32 StartingCheatIndex = FCheatSheetNamespace::MaxCheatPageRows * CurrentCheatPage;
	const int32 EndingCheatIndex = StartingCheatIndex + FCheatSheetNamespace::MaxCheatPageRows;
	const int32 EndingCheatIndexSafe = EndingCheatIndex <= DataPack.ConsoleVariableNames.Num()
	? EndingCheatIndex
	: DataPack.ConsoleVariableNames.Num();
	
	for(int32 i = StartingCheatIndex; i < EndingCheatIndexSafe - 1; ++i)
	{
		float StrCheatWidth = 0.0f, StrCheatHeight = 0.0f;
		CanvasContext.MeasureString(DataPack.ConsoleVariableNames[i], StrCheatWidth, StrCheatHeight);
		FString Spaces = AppendCharMult(MaxCheatWidth - StrCheatWidth, ' ', CanvasContext, 5);
		CanvasContext.Printf(TEXT("%ls %s {white}%ls"), *DataPack.ConsoleVariableNames[i], *Spaces, *DataPack.ConsoleVariableDescriptions[i]);
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
	CurrentCheatPage++;
	CurrentCheatPage = CurrentCheatPage <= MaxPagesCount ? CurrentCheatPage++ : 0;
	UE_LOG(LogTemp, Warning, TEXT("NextPage Draw, CurrentCheatPage: %i"), CurrentCheatPage);
}

void FCheatSheet_GameplayDebuggerCategory::DrawPrevPage()
{
	CurrentCheatPage--;
	CurrentCheatPage = CurrentCheatPage >= 0 ? CurrentCheatPage : MaxPagesCount;
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

FString FCheatSheet_GameplayDebuggerCategory::AppendCharMult(float InWidth, const TCHAR& Char, const FGameplayDebuggerCanvasContext& CanvasContext, int32 AdditionalWidth)
{
	FString OutString;
	float CharWidth = 0.0f, CharHeight = 0.0f;
	CanvasContext.MeasureString(FString().AppendChar(Char), CharWidth, CharHeight);
	float CountFlt = 0.0f;
	FMath::Modf(InWidth / CharWidth, &CountFlt);
	const int32 CountInt = static_cast<int32>(CountFlt) + AdditionalWidth;
	for (int i = 0; i < CountInt; ++i)
	{
		OutString.AppendChar(Char);	
	}
	return OutString;
}

bool FCheatSheet_GameplayDebuggerCategory::WrapStringAccordingToViewport(const FString& StrIn, FString& StrOut,	FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWitdh)
{
	if (!StrIn.IsEmpty())
	{
		// Clamp the Width
		ViewportWitdh = FMath::Max(ViewportWitdh, 10.0f);

		float StrWidth = 0.0f, StrHeight = 0.0f;
		// Calculate the length(in pixel) of the tags
		CanvasContext.MeasureString(StrIn, StrWidth, StrHeight);

		int32 SubDivision = FMath::CeilToInt(StrWidth / ViewportWitdh);
		if (SubDivision > 1)
		{
			// Copy the string
			StrOut = StrIn;
			const int32 Step = StrOut.Len() / SubDivision;
			// Start sub divide if needed
			for (int32 i = SubDivision - 1; i > 0; --i)
			{
				// Insert Line Feed
				StrOut.InsertAt(i * Step - 1, '\n');
			}
			return true;
		}
	}
	// No need to wrap the text 
	return false;
}