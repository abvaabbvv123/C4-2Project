// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4ScoreRowWidget.h"
#include "Components/TextBlock.h"

void UC4ScoreRowWidget::SetRowData(const FString& InPlayerName, int32 InScore, bool bIsLocalPlayer)
{
	if (CachedName != InPlayerName)
	{
		CachedName = InPlayerName;
		TextBlock_PlayerName->SetText(FText::FromString(InPlayerName));
	}

	if (CachedScore != InScore)
	{
		CachedScore = InScore;
		TextBlock_Score->SetText(FText::AsNumber(InScore));
	}

	const int8 NewIsLocal = bIsLocalPlayer ? 1 : 0;
	if (CachedIsLocal != NewIsLocal)
	{
		CachedIsLocal = NewIsLocal;
		const FSlateColor& Color = bIsLocalPlayer ? LocalPlayerColor : OtherPlayerColor;
		TextBlock_PlayerName->SetColorAndOpacity(Color);
		TextBlock_Score->SetColorAndOpacity(Color);
	}
}