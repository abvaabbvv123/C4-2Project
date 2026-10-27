// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4GameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
void AC4GameState::OnRep_MatchState() { OnMatchStateChanged.Broadcast(MatchState); }
void AC4GameState::OnRep_RemainingTime() { OnRemainingTimeChanged.Broadcast(RemainingTime); }
void AC4GameState::OnRep_Win() { OnWin.Broadcast(); }

void AC4GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, MatchState);
	DOREPLIFETIME(ThisClass, RemainingTime);
	DOREPLIFETIME(ThisClass, Winner);
}

void AC4GameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	if (PlayerArray.Contains(PlayerState))
	{
		OnPlayerAdded.Broadcast(PlayerState);
	}
}
void AC4GameState::RemovePlayerState(APlayerState* PlayerState)
{
	OnPlayerRemoved.Broadcast(PlayerState);
	Super::RemovePlayerState(PlayerState);
}
void AC4GameState::SetMatchState(EC4MatchState NewState)
{
	if (!HasAuthority() || MatchState == NewState) return;
	MatchState = NewState;
	ForceNetUpdate();
	OnRep_MatchState();
}
void AC4GameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority() || RemainingTime == NewTime) return;
	RemainingTime = NewTime;
	ForceNetUpdate();
	OnRep_RemainingTime();
}
void AC4GameState::SetWinners(const TArray<TObjectPtr<APlayerState>>& NewWinners)
{
	if (!HasAuthority()) return;
	Winner = NewWinners;
	ForceNetUpdate();
	OnRep_Win();
}


