// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4ScoreRowWidget.generated.h"

class UTextBlock;
//class APlayerState;
//class AC4PlayerState;
/**
 * 
 */
UCLASS()
class CX_API UC4ScoreRowWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetRowData(const FString& InPlayerName, int32 InScore, bool bIsLocalPlayer);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_PlayerName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Score;

	UPROPERTY(EditDefaultsOnly, Category = "Scoreboard")
	FSlateColor LocalPlayerColor = FSlateColor(FLinearColor(0.3f, 1.f, 0.3f));

	UPROPERTY(EditDefaultsOnly, Category = "Scoreboard")
	FSlateColor OtherPlayerColor = FSlateColor(FLinearColor::White);


	FString CachedName;
	int32 CachedScore = INDEX_NONE;
	int8 CachedIsLocal = -1;
};