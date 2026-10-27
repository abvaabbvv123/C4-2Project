// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4MatchWidget.h"
#include "System/C4GameState.h"
#include "System/C4PlayerState.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

void UC4MatchWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Panel_Result->SetVisibility(ESlateVisibility::Collapsed);

	if (UWorld* World = GetWorld())
	{
		GSSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::HandleGS );
		HandleGS(World->GetGameState());
	}
}

void UC4MatchWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GSSetHandle);
	}
	UnbindGS();
	Super::NativeDestruct();
}

void UC4MatchWidget::HandleGS(AGameStateBase* NewGameState)
{
	BindGS(Cast<AC4GameState>(NewGameState));
}
void UC4MatchWidget::BindGS(AC4GameState* NewGameState)
{
	if (!IsValid(NewGameState) || BoundGS.Get() == NewGameState) return;
	UnbindGS();

	BoundGS = NewGameState;
	NewGameState->OnMatchStateChanged.AddUObject(this, &ThisClass::HandleMatchStateChanged);
	NewGameState->OnRemainingTimeChanged.AddUObject(this, &ThisClass::HandleRemainingTimeChanged);
	NewGameState->OnWin.AddUObject(this, &ThisClass::RefreshResult);

	HandleMatchStateChanged(NewGameState->GetMatchState());
}

void UC4MatchWidget::UnbindGS()
{
	if (AC4GameState* GS = BoundGS.Get())
	{
		GS->OnMatchStateChanged.RemoveAll(this);
		GS->OnRemainingTimeChanged.RemoveAll(this);
		GS->OnWin.RemoveAll(this);
	}
	BoundGS.Reset();
}
void UC4MatchWidget::HandleMatchStateChanged(EC4MatchState NewState)
{
	NowState = NewState;
	const bool bFinished = (NewState == EC4MatchState::Finished);

	TextBlock_Status->SetVisibility(bFinished ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	Panel_Result->SetVisibility(bFinished ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (bFinished) RefreshResult();
	if (const AC4GameState* GS = BoundGS.Get())
	{
		HandleRemainingTimeChanged(GS->GetRemainingTime());
	}

	OnMatchStateChange(NewState, bFinished && IsLocalWinner());
}
void UC4MatchWidget::HandleRemainingTimeChanged(int32 NewTime)
{
	switch (NowState)
	{
	case EC4MatchState::Waiting:
		// 0 = waiting arrival, over than 0 = Start
		TextBlock_Status->SetText(NewTime > 0
			? C4Text::StartingIn(NewTime)
			: C4Text::WaitingPlayers());
		//TextBlock_Status->SetColorAndOpacity(NormalColor);
		break;

	case EC4MatchState::Playing:
		TextBlock_Status->SetText(C4Text::MatchTime(NewTime));
	//	TextBlock_Status->SetColorAndOpacity(NewTime <= WarningSeconds ? WarningColor : NormalColor);
		break;

	case EC4MatchState::Finished:
		TextBlock_ReturnTime->SetText(C4Text::ReturningIn(NewTime));
		break;
	}
}
void UC4MatchWidget::RefreshResult()
{
	const AC4GameState* GS = BoundGS.Get();
	if (!GS || NowState != EC4MatchState::Finished) return;

	const TArray<TObjectPtr<APlayerState>>& Winners = GS->GetWinners();
	const bool bWin = IsLocalWinner();
	TextBlock_ResultTitle->SetText(bWin ? (Winners.Num() > 1 ? C4Text::ResultDraw() : C4Text::ResultWin()) : C4Text::ResultLose());

	TArray<FString> Lines;
	for (const APlayerState* PS : Winners)
	{
		if (const AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS))
		{
			Lines.Add(C4Text::WinnerEntry(C4PS->GetPlayerName(), C4PS->GetTotalScore()).ToString());
		}
	}
	TextBlock_Winners->SetText(Lines.Num() > 0 ? C4Text::Winners(Lines) : C4Text::NoWinner());
}

bool UC4MatchWidget::IsLocalWinner() const
{
	const AC4GameState* GS = BoundGS.Get();
	return GS && GS->IsWinner(GetOwningPlayerState());
}