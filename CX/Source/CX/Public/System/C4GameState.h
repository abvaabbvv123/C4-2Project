// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Base/C4Type.h"
#include "C4GameState.generated.h"
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerArrayChanged, APlayerState*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMatchChanged, EC4MatchState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, int32);
DECLARE_MULTICAST_DELEGATE(FOnWin);
/**
 * 
 */
UCLASS()
class CX_API AC4GameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	void SetMatchState(EC4MatchState NewState);
	void SetRemainingTime(int32 NewTime);
	void SetWinners(const TArray<TObjectPtr<APlayerState>>& NewWinners);

	EC4MatchState GetMatchState() const { return MatchState; }
	int32 GetRemainingTime() const { return RemainingTime; }
	bool IsPlaying() const { return MatchState == EC4MatchState::Playing; }
	const TArray<TObjectPtr<APlayerState>>& GetWinners() const { return Winner; }
	bool IsWinner(APlayerState* PS) const { return PS && Winner.Contains(PS); }

	FOnPlayerArrayChanged OnPlayerAdded;
	FOnPlayerArrayChanged OnPlayerRemoved;
	FOnMatchChanged OnMatchStateChanged;
	FOnTimeChanged OnRemainingTimeChanged;
	FOnWin OnWin;
protected:
	UFUNCTION() void OnRep_MatchState();
	UFUNCTION() void OnRep_RemainingTime();
	UFUNCTION() void OnRep_Win();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	EC4MatchState MatchState = EC4MatchState::Waiting;
	UPROPERTY(ReplicatedUsing = OnRep_Win)
	TArray<TObjectPtr<APlayerState>> Winner;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	int32 RemainingTime = 0;
};