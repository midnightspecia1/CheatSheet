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
	TArray<FString> CheatKeyWords;

	TPair<float, float> EvalLongestString(const TArray<FString>& Strings, const FGameplayDebuggerCanvasContext& CanvasContext);
	float EvalStringLength(const FString& String, const FGameplayDebuggerCanvasContext& CanvasContext);
};

#endif //WITH_GAMEPLAY_DEBUGGER