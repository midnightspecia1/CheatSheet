// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "UObject/Object.h"

/**
 * 
 */

class CHEATSHEET_API FCheatSheet_GameplayDebuggerCategory final : public FGameplayDebuggerCategory
{
public:
	
    FCheatSheet_GameplayDebuggerCategory();
    virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:
	struct FRepData
	{
		TArray<FString> ConsoleVariableNames;
		TArray<FString> ConsoleVariableDescriptions;

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;

	void DrawNextPage();
	void DrawPrevPage();
	
private:
	bool bDataCollected = false;
	TArray<FString> CheatKeyWords;

	int32 CurrentCheatPage = 0;
	int32 MaxPagesCount = 0;
	
	FString AppendCharMult(float InWidth, const TCHAR& Char, const FGameplayDebuggerCanvasContext& CanvasContext, int32 AdditionalWidth);
	bool WrapStringAccordingToViewport(const FString& StrIn, FString& StrOut, FGameplayDebuggerCanvasContext& CanvasContext, float ViewportWitdh);
};

#endif //WITH_GAMEPLAY_DEBUGGER