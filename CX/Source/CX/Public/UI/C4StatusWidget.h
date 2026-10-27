// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4StatusWidget.generated.h"
class UTextBlock;
class APlayerState;
class AC4PlayerState;

/**
 * 
 */
UCLASS()
class CX_API UC4StatusWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandlePlayerStateSet(APlayerState* NewPlayerState);
	void BindPlayerState(AC4PlayerState* NewPlayerState);
	void UnbindPlayerState();

	void HandleCoinsChanged(int32 Amount);
	void HandleCurrentBonusChanged(int32 Amount);
	void HandleMineChanged(int32 Amount);

	UPROPERTY(meta = (BindWidget))	TObjectPtr<UTextBlock> TextBlock_Coins;
	UPROPERTY(meta = (BindWidget))	TObjectPtr<UTextBlock> TextBlock_BonusSpeed;
	UPROPERTY(meta = (BindWidget))	TObjectPtr<UTextBlock> TextBlock_MineCount;

	TWeakObjectPtr<AC4PlayerState> BoundPlayerState;
};