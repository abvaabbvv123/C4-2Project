// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4ScoreBoardWidget.generated.h"

class UVerticalBox;
class UC4ScoreRowWidget;
class AGameStateBase;
class AC4GameState;
class APlayerState;
class AC4PlayerState;

/**
 * 
 */
UCLASS()
class CX_API UC4ScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	void HandleGameStateSet(AGameStateBase* NewGameState);
	void HandleOwnerPlayerStateSet(APlayerState* NewPlayerState);
	void BindGameState(AC4GameState* NewGameState);
	void UnbindGameState();

	void HandlePlayerAdded(APlayerState* PlayerState);
	void HandlePlayerRemoved(APlayerState* PlayerState);

	void BindPlayerState(AC4PlayerState* PlayerState);
	void UnbindPlayerState(AC4PlayerState* PlayerState);
	void UnbindAllPlayerStates();

	void HandleScoreChanged(int32 NewScore);

	void RequestRefresh();
	void RefreshRows();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_Rows;
	UPROPERTY(EditDefaultsOnly, Category = "Scoreboard")
	TSubclassOf<UC4ScoreRowWidget> RowWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UC4ScoreRowWidget>> Rows;

	TWeakObjectPtr<AC4GameState> BoundGameState;
	TArray<TWeakObjectPtr<AC4PlayerState>> BoundPlayerStates;

	FDelegateHandle GameStateSetHandle;
	bool bRefreshPending = false;
};