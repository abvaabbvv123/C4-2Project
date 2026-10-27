// Fill out your copyright notice in the Description page of Project Settings.



#include "UI/C4ScoreBoardWidget.h"
#include "UI/C4ScoreRowWidget.h"
#include "System/C4GameState.h"
#include "System/C4PlayerState.h"
#include "System/C4PlayerController.h" 
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Algo/Sort.h"

void UC4ScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AC4PlayerController* PC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
		PC->OnPSSet.AddUObject(this, &ThisClass::HandleOwnerPlayerStateSet);
	}

	if (UWorld* World = GetWorld())
	{
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::HandleGameStateSet);
		HandleGameStateSet(World->GetGameState());
	}
}


void UC4ScoreBoardWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	if (AC4PlayerController* PC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
	    PC->OnPSSet.RemoveAll(this);
	}

	UnbindAllPlayerStates();   // PlayerState 구독 먼저
	UnbindGameState();
	bRefreshPending = false;

	Super::NativeDestruct();
}

void UC4ScoreBoardWidget::HandleGameStateSet(AGameStateBase* NewGameState)
{
	BindGameState(Cast<AC4GameState>(NewGameState));
}
void UC4ScoreBoardWidget::HandleOwnerPlayerStateSet(APlayerState* /*NewPlayerState*/)
{
	RequestRefresh();
}
void UC4ScoreBoardWidget::BindGameState(AC4GameState* NewGameState)
{
	if (BoundGameState.Get() == NewGameState) return;
	if (!IsValid(NewGameState)) return;
	UnbindAllPlayerStates();
	UnbindGameState();

	BoundGameState = NewGameState;
	NewGameState->OnPlayerAdded.AddUObject(this, &ThisClass::HandlePlayerAdded);
	NewGameState->OnPlayerRemoved.AddUObject(this, &ThisClass::HandlePlayerRemoved);
	for (APlayerState* PS : NewGameState->PlayerArray)
	{
		BindPlayerState(Cast<AC4PlayerState>(PS));
	}
	RequestRefresh();
}

void UC4ScoreBoardWidget::UnbindGameState()
{
	if (AC4GameState* GS = BoundGameState.Get())
	{
		GS->OnPlayerAdded.RemoveAll(this);
		GS->OnPlayerRemoved.RemoveAll(this);
	}
	BoundGameState.Reset();
}

void UC4ScoreBoardWidget::HandlePlayerAdded(APlayerState* PlayerState)
{
	BindPlayerState(Cast<AC4PlayerState>(PlayerState));
	RequestRefresh();
}

void UC4ScoreBoardWidget::HandlePlayerRemoved(APlayerState* PlayerState)
{
	UnbindPlayerState(Cast<AC4PlayerState>(PlayerState));
	RequestRefresh();
}

void UC4ScoreBoardWidget::BindPlayerState(AC4PlayerState* PlayerState)
{
	if (!IsValid(PlayerState) || BoundPlayerStates.Contains(PlayerState))return;

	PlayerState->OnScoreChanged.AddUObject(this, &ThisClass::HandleScoreChanged);
	PlayerState->OnPlayerNameUpdated.AddUObject(this, &ThisClass::RequestRefresh);
	BoundPlayerStates.Add(PlayerState);
}
void UC4ScoreBoardWidget::UnbindPlayerState(AC4PlayerState* PlayerState)
{
	if (!PlayerState) return;
	PlayerState->OnScoreChanged.RemoveAll(this);
	PlayerState->OnPlayerNameUpdated.RemoveAll(this);
	BoundPlayerStates.Remove(PlayerState);
}
void UC4ScoreBoardWidget::UnbindAllPlayerStates()
{
	for (const TWeakObjectPtr<AC4PlayerState>& PS : BoundPlayerStates)
	{
		if (AC4PlayerState* C4PS = PS.Get())
		{
			PS->OnScoreChanged.RemoveAll(this);
			PS->OnPlayerNameUpdated.RemoveAll(this);
		}
	}
	BoundPlayerStates.Empty();
}

void UC4ScoreBoardWidget::HandleScoreChanged(int32 NewScore)
{
	RequestRefresh();
}
// request only once even if call mani times in one fream
void UC4ScoreBoardWidget::RequestRefresh()
{
	if (bRefreshPending) return;
	if (UWorld* World = GetWorld())
	{
		bRefreshPending = true;
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RefreshRows);
	}
}
void UC4ScoreBoardWidget::RefreshRows()
{
	bRefreshPending = false;

	AC4GameState* GS = BoundGameState.Get();
	if (!GS || !RowWidgetClass) return;
	//get players
	TArray<AC4PlayerState*> SortedPlayers;
	SortedPlayers.Reserve(GS->PlayerArray.Num());
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS); IsValid(C4PS))
		{
			SortedPlayers.Add(C4PS);
		}
	}
	//sorting
	Algo::Sort(SortedPlayers, [](const AC4PlayerState* A, const AC4PlayerState* B)
		{
			if (A->GetTotalScore() != B->GetTotalScore())
			{
				return A->GetTotalScore() > B->GetTotalScore();
			}
			return A->GetPlayerId() < B->GetPlayerId();
		});
	//extending rows
	while (Rows.Num() < SortedPlayers.Num())
	{
		UC4ScoreRowWidget* NewRow = CreateWidget<UC4ScoreRowWidget>(this, RowWidgetClass);
		if (!NewRow) return;
		Rows.Add(NewRow);
		VerticalBox_Rows->AddChildToVerticalBox(NewRow);
	}
	//show total score
	const APlayerState* LocalPS = GetOwningPlayerState();
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		UC4ScoreRowWidget* Row = Rows[i];
		if (i < SortedPlayers.Num())
		{
			const AC4PlayerState* C4PS = SortedPlayers[i];
			Row->SetRowData(C4PS->GetPlayerName(), C4PS->GetTotalScore(), C4PS == LocalPS);
			Row->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Row->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}