// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "C4PlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnValueChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, bool);
/**
 * 
 */
UCLASS()
class CX_API AC4PlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void ResetMatchStats();

	void AddCoins(int32 Amount);
	bool UseCoins(int32 Amount);
	int32 LoseCoins(int32 Amount);
	int32 StealCoins(AC4PlayerState* Player, int32 Amount);
	void ChangeBonusSpeed(int32 Amount);
	void ApplySpeedPenalty(int32 Amount);
	bool AddMine();
	bool UseMine();

	void SetReady(bool bNewReady);
	bool IsReady() const { return bReady; }

	//Getter
	int32 GetCoins() const { return Coins; }
	int32 GetCoinScore() const { return CoinScore; }
	int32 GetStealScore() const { return StealScore; }
    int32 GetTotalScore() const { return CoinScore + StealScore; }
	int32 GetBonusSpeed() const { return BonusSpeed; }
	int32 GetCurrentBonusSpeed() const { return CurrentBonusSpeed; }
	int32 GetMineCount() const { return MineCount; }

	FOnValueChanged OnCoinsChanged;
	FOnValueChanged OnScoreChanged;
	FOnValueChanged OnBonusSpeedChanged;
	FOnValueChanged OnCurrentBonusSpeedChanged;
	FOnValueChanged OnMineCountChanged;
	FOnReadyChanged OnReadyChanged;
	FSimpleMulticastDelegate OnPlayerNameUpdated;

protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;
	virtual void OnRep_PlayerName() override;
	//onreps
	UFUNCTION() void OnRep_Coins();
	UFUNCTION() void OnRep_CoinScore();
	UFUNCTION() void OnRep_StealScore();
	UFUNCTION() void OnRep_BonusSpeed();
	UFUNCTION() void OnRep_CurrentBonusSpeed();
	UFUNCTION() void OnRep_MineCount();
	UFUNCTION() void OnRep_Ready();

	UPROPERTY(ReplicatedUsing = OnRep_Coins) int32 Coins = 0;
	UPROPERTY(ReplicatedUsing = OnRep_CoinScore) int32 CoinScore = 0;
	UPROPERTY(ReplicatedUsing = OnRep_StealScore) int32 StealScore = 0;
	UPROPERTY(ReplicatedUsing = OnRep_BonusSpeed) int32 BonusSpeed = 0;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentBonusSpeed) int32 CurrentBonusSpeed = 0;
	UPROPERTY(ReplicatedUsing = OnRep_MineCount) int32 MineCount = 0;
	UPROPERTY(ReplicatedUsing = OnRep_Ready) bool bReady = false;

	UPROPERTY(EditDefaultsOnly, Category = "Score") int32 CoinScoreAmount = 5;
	UPROPERTY(EditDefaultsOnly, Category = "Score") int32 StealScoreAmount = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Speed") int32 MaxBonusSpeed = 100;
	UPROPERTY(EditDefaultsOnly, Category = "Speed") int32 MinBonusSpeed = -95;
	UPROPERTY(EditDefaultsOnly, Category = "Speed") int32 RecoveryStep = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Speed") float  RecoveryInterval = 0.1f;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") int32 MaxMineCount = 5;
private:
	void SetCurrentBonusSpeed(int32 value);
	void StartRecovery();
	void TickRecovery();

	FTimerHandle RecoveryTimerHandle; // for recovery speed penalty

};
