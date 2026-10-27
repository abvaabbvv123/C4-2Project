// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Base/C4Type.h"
#include "C4MatchWidget.generated.h"
class UTextBlock;
class UWidget;
class AGameStateBase;
class AC4GameState;
/**
 * 
 */
UCLASS()
class CX_API UC4MatchWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	
	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnMatchStateChange(EC4MatchState NewState, bool bLocalWin);

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_Status;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UWidget>    Panel_Result;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_ResultTitle;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_Winners;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_ReturnTime;

	// 표시 문구는 C4Type.h의 C4Text에서 관리

private:
	void HandleGS(AGameStateBase* NewGameState);
	void BindGS(AC4GameState* NewGameState);
	void UnbindGS();

	void HandleMatchStateChanged(EC4MatchState NewState);
	void HandleRemainingTimeChanged(int32 NewTime);
	void RefreshResult();
	bool IsLocalWinner() const;

	TWeakObjectPtr<AC4GameState> BoundGS;
	FDelegateHandle GSSetHandle;
	EC4MatchState NowState = EC4MatchState::Waiting;
};
