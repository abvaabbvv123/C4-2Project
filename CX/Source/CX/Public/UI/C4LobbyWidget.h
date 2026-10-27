// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4LobbyWidget.generated.h"
class UVerticalBox;
class UTextBlock;
class UButton;
class AGameStateBase;
class AC4GameState;
class APlayerState;
class AC4PlayerState;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class CX_API UC4LobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEditableTextBox> UEditableTextBox_NickName;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UEditableTextBox> UEditableTextBox_Server;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_JoinServer;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_ConfirmNickName;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UVerticalBox> VerticalBox_Players;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_ServerIP;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_LobbyStatus;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_ReadyLabel;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_Message;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Ready;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Leave;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Copy;

	UFUNCTION() void OnCopyClicked();
	UFUNCTION() void OnLeaveClicked();
	UFUNCTION() void OnReadyClicked();
	UFUNCTION() void OnNickNameClicked();
	UFUNCTION() void OnServerClicked();

	UPROPERTY(EditDefaultsOnly, Category = "Lobby") int32 RequiredPlayers = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") FSlateFontInfo RowFont = FCoreStyle::GetDefaultFontStyle("Regular", 18);
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") FSlateColor ReadyColor = FSlateColor(FLinearColor(0.3f, 1.f, 0.4f));
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") FSlateColor NotReadyColor = FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));

private:

	void ShowInfo(const FText& Message, bool bError);
	void HandleGS_Lobby(AGameStateBase* NewGameState);
	void BindGS_Lobby(AC4GameState* NewGameState);
	void UnbindGS_Lobby();
	void HandlePlayerJoin(APlayerState* PlayerState);
	void HandlePlayerLeave(APlayerState* PlayerState);
	void BindPS_Lobby(AC4PlayerState* PlayerState);
	void UnbindPS_Lobby(AC4PlayerState* PlayerState);
	void UnbindAllPS_Lobby();

	void HandleReadyChanged(bool bReady) { RequestRefresh(); }
	void HandleRemainingTimeChanged(int32) { RequestRefresh(); }
	void HandleInfo(const FText& Text) { ShowInfo(Text, false); }

	void RequestRefresh();
	void Refresh();

	TWeakObjectPtr<AC4GameState> BoundGameState;
	TArray<TWeakObjectPtr<AC4PlayerState>> BoundPlayerStates;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> RowTexts;

	FDelegateHandle GameStateSetHandle;
	bool bRefreshPending = false;
};