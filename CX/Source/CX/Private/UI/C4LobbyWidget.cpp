// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4LobbyWidget.h"
#include "System/C4GameState.h"
#include "System/C4PlayerState.h"
#include "System/C4LobbyPlayerController.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
#include "System/C4GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/EditableTextBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Base/C4Type.h"
void UC4LobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Button_Ready->OnClicked.AddUniqueDynamic(this, &ThisClass::OnReadyClicked);
	Button_Leave->OnClicked.AddUniqueDynamic(this, &ThisClass::OnLeaveClicked);
	Button_ConfirmNickName->OnClicked.AddUniqueDynamic(this, &ThisClass::OnNickNameClicked);
	Button_JoinServer->OnClicked.AddUniqueDynamic(this, &ThisClass::OnServerClicked);
	Button_Copy->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCopyClicked);

	ShowInfo(FText::GetEmpty(), false);
	if (UC4GameInstance* GI = GetGameInstance<UC4GameInstance>())
	{
		UEditableTextBox_NickName->SetText(FText::FromString(GI->GetNickname()));

		const FString Code = GI->GetRoomCode();
		TextBlock_ServerIP->SetText(C4Text::RoomCode(Code.IsEmpty() ? FString() : UC4GameInstance::FormatRoomCode(Code)));

		const FString Error = GI->ChangeLastError();
		if (!Error.IsEmpty())
		{
			ShowInfo(C4Text::JoinFailed(Error), true);
		}
	}
	if (AC4PlayerController* PC = GetOwningPlayer<AC4PlayerController>())
	{
		PC->OnInfo.AddUObject(this, &ThisClass::HandleInfo);
	}
	if (UWorld* World = GetWorld())
	{
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::HandleGS_Lobby);
		HandleGS_Lobby(World->GetGameState());
	}
}

void UC4LobbyWidget::NativeDestruct()
{
	Button_Ready->OnClicked.RemoveDynamic(this, &ThisClass::OnReadyClicked);
	Button_Leave->OnClicked.RemoveDynamic(this, &ThisClass::OnLeaveClicked);
	Button_ConfirmNickName->OnClicked.RemoveDynamic(this, &ThisClass::OnNickNameClicked);
	Button_JoinServer->OnClicked.RemoveDynamic(this, &ThisClass::OnServerClicked);
	Button_Copy->OnClicked. RemoveDynamic(this, &ThisClass::OnCopyClicked); 

	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	if (AC4PlayerController* PC = GetOwningPlayer<AC4PlayerController>())
	{
		PC->OnInfo.RemoveAll(this);
	}
	UnbindAllPS_Lobby();
	UnbindGS_Lobby();
	bRefreshPending = false;
	Super::NativeDestruct();
}
void UC4LobbyWidget::OnCopyClicked()
{
	const UC4GameInstance* GI = GetGameInstance<UC4GameInstance>();
	const FString Code = GI ? GI->GetRoomCode() : FString();
	if (Code.IsEmpty())
	{
		ShowInfo(C4Text::NoRoomCode(), true);
		return;
	}
	FPlatformApplicationMisc::ClipboardCopy(*UC4GameInstance::FormatRoomCode(Code));
	ShowInfo(C4Text::CodeCopied(), false);
}

void UC4LobbyWidget::OnNickNameClicked()
{
	const FString Name = UEditableTextBox_NickName->GetText().ToString().TrimStartAndEnd();
	if (Name.IsEmpty())
	{
		ShowInfo(C4Text::NicknameEmpty(), true);
		return;
	}
	if (AC4LobbyPlayerController* PC = GetOwningPlayer<AC4LobbyPlayerController>())
	{
		PC->RequestNickname(Name);
		ShowInfo(C4Text::NicknameUpdated(), false);
	}
}
void UC4LobbyWidget::OnServerClicked()
{
	UC4GameInstance* GI = GetGameInstance<UC4GameInstance>();
	if (!GI) return;

	FString Error;
	if (!GI->JoinServer(UEditableTextBox_Server->GetText().ToString(), Error))
	{
		ShowInfo(FText::FromString(Error), true);
		return;
	}
	Button_JoinServer->SetIsEnabled(false);
	ShowInfo(C4Text::JoiningRoom(), false);
}

void UC4LobbyWidget::ShowInfo(const FText& Message, bool bError)
{
	if (!TextBlock_Message) return;
	TextBlock_Message->SetText(Message);
}
void UC4LobbyWidget::OnReadyClicked()
{
	if (AC4LobbyPlayerController* PC = GetOwningPlayer<AC4LobbyPlayerController>())
	{
		PC->ToggleReady();
	}
}
void UC4LobbyWidget::OnLeaveClicked()
{
	if (UC4GameInstance* GI = GetGameInstance<UC4GameInstance>())
	{
		GI->ToTitle();
	}
}
void UC4LobbyWidget::HandleGS_Lobby(AGameStateBase* NewGameState)
{
	BindGS_Lobby(Cast<AC4GameState>(NewGameState));
}

void UC4LobbyWidget::BindGS_Lobby(AC4GameState* NewGameState)
{
	if (!IsValid(NewGameState) || BoundGameState.Get() == NewGameState) return;
	UnbindAllPS_Lobby();
	UnbindGS_Lobby();

	BoundGameState = NewGameState;
	NewGameState->OnPlayerAdded.AddUObject(this, &ThisClass::HandlePlayerJoin);
	NewGameState->OnPlayerRemoved.AddUObject(this, &ThisClass::HandlePlayerLeave);
	NewGameState->OnRemainingTimeChanged.AddUObject(this, &ThisClass::HandleRemainingTimeChanged);

	for (APlayerState* PS : NewGameState->PlayerArray)
	{
		BindPS_Lobby(Cast<AC4PlayerState>(PS));
	}
	RequestRefresh();
}

void UC4LobbyWidget::UnbindGS_Lobby()
{
	if (AC4GameState* GS = BoundGameState.Get())
	{
		GS->OnPlayerAdded.RemoveAll(this);
		GS->OnPlayerRemoved.RemoveAll(this);
		GS->OnRemainingTimeChanged.RemoveAll(this);
	}
	BoundGameState.Reset();
}

void UC4LobbyWidget::HandlePlayerJoin(APlayerState* PlayerState)
{
	BindPS_Lobby(Cast<AC4PlayerState>(PlayerState));
	RequestRefresh();
}

void UC4LobbyWidget::HandlePlayerLeave(APlayerState* PlayerState)
{
	UnbindPS_Lobby(Cast<AC4PlayerState>(PlayerState));
	RequestRefresh();
}

void UC4LobbyWidget::BindPS_Lobby(AC4PlayerState* PlayerState)
{
	if (!IsValid(PlayerState) || BoundPlayerStates.Contains(PlayerState)) return;
	PlayerState->OnReadyChanged.AddUObject(this, &ThisClass::HandleReadyChanged);
	PlayerState->OnPlayerNameUpdated.AddUObject(this, &ThisClass::RequestRefresh);
	BoundPlayerStates.Add(PlayerState);
}

void UC4LobbyWidget::UnbindPS_Lobby(AC4PlayerState* PlayerState)
{
	if (!PlayerState) return;
	PlayerState->OnReadyChanged.RemoveAll(this);
	PlayerState->OnPlayerNameUpdated.RemoveAll(this);
	BoundPlayerStates.Remove(PlayerState);
}

void UC4LobbyWidget::UnbindAllPS_Lobby()
{
	for (const TWeakObjectPtr<AC4PlayerState>& Weak : BoundPlayerStates)
	{
		if (AC4PlayerState* PS = Weak.Get())
		{
			PS->OnReadyChanged.RemoveAll(this);
			PS->OnPlayerNameUpdated.RemoveAll(this);
		}
	}
	BoundPlayerStates.Empty();
}

void UC4LobbyWidget::RequestRefresh()
{
	if (bRefreshPending) return;
	if (UWorld* World = GetWorld())
	{
		bRefreshPending = true;
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::Refresh);
	}
}
void UC4LobbyWidget::Refresh()
{
	bRefreshPending = false;
	const AC4GameState* GS = BoundGameState.Get();
	if (!GS) return;

	const APlayerState* LocalPS = GetOwningPlayerState();
	int32 Count = 0;
	int32 ReadyCount = 0;
	bool bLocalReady = false;

	for (APlayerState* PS : GS->PlayerArray)
	{
		const AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS);
		if (!IsValid(C4PS)) continue;
		if (!RowTexts.IsValidIndex(Count))
		{
			UTextBlock* NewRow = WidgetTree->ConstructWidget<UTextBlock>();
			NewRow->SetFont(RowFont);
			VerticalBox_Players->AddChildToVerticalBox(NewRow);
			RowTexts.Add(NewRow);
		}

		const bool bReady = C4PS->IsReady();
		const bool bLocal = (C4PS == LocalPS);
		UTextBlock* Row = RowTexts[Count];
		Row->SetText(C4Text::PlayerRow(C4PS->GetPlayerName(), bLocal, bReady));
		Row->SetColorAndOpacity(bReady ? ReadyColor : NotReadyColor);
		Row->SetVisibility(ESlateVisibility::HitTestInvisible);

		++Count;
		if (bReady) ++ReadyCount;
		if (bLocal) bLocalReady = bReady;
	}

	for (int32 i = Count; i < RowTexts.Num(); ++i)
	{
		RowTexts[i]->SetVisibility(ESlateVisibility::Collapsed);
	}

	TextBlock_ReadyLabel->SetText(C4Text::ReadyButton(bLocalReady));

	const int32 Countdown = GS->GetRemainingTime();
	FText Status;
	if (Countdown > 0)                 Status = C4Text::LobbyStartingIn(Countdown);
	else if (Count < RequiredPlayers)  Status = C4Text::LobbyWaiting(Count, RequiredPlayers);
	else                               Status = C4Text::LobbyReadyCount(ReadyCount, Count);
	TextBlock_LobbyStatus->SetText(Status);
}