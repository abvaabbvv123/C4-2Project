// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

void AC4PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//owneronly
	DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, BonusSpeed, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentBonusSpeed, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, MineCount, COND_OwnerOnly);
	//all
	DOREPLIFETIME(ThisClass, CoinScore);
	DOREPLIFETIME(ThisClass, StealScore);
	DOREPLIFETIME(ThisClass, bReady);
}
void AC4PlayerState::ResetMatchStats()
{
	if (!HasAuthority()) return;
	Coins = 0;
	CoinScore = 0;
	StealScore = 0;
	BonusSpeed = 0;
	CurrentBonusSpeed = 0;
	MineCount = 0;
	ForceNetUpdate();

	OnRep_Coins();
	OnRep_CoinScore();
	OnRep_BonusSpeed();
	OnRep_CurrentBonusSpeed();
	OnRep_MineCount();
}
void AC4PlayerState::AddCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0) return;

	Coins += Amount;
	CoinScore += Amount * CoinScoreAmount;
	ForceNetUpdate();

	OnRep_Coins();
	OnRep_CoinScore();
}
bool AC4PlayerState::UseCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Coins < Amount) return false;

	Coins -= Amount;
	ForceNetUpdate();

	OnRep_Coins();
	return true;
}
int32 AC4PlayerState::LoseCoins(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Coins <= 0) return 0;
	const int32 Lost = FMath::Min(Amount, Coins);
	Coins -= Lost;
	ForceNetUpdate();
	OnRep_Coins();
	return Lost;
}
int32 AC4PlayerState::StealCoins(AC4PlayerState* Player, int32 Amount) 
{
	if (!HasAuthority() || !IsValid(Player) || Player == this || Amount <= 0) return 0;
	const int32 Stolen = Player->LoseCoins(Amount);
	if (!Stolen || Stolen <= 0) return 0;
	Coins += Stolen;
	StealScore += Stolen * StealScoreAmount;
	ForceNetUpdate();

	OnRep_Coins();
	OnRep_StealScore();
	return Stolen;
}
void AC4PlayerState::SetReady(bool bNewReady)
{
	if (!HasAuthority() || bReady == bNewReady) return;
	bReady = bNewReady;
	ForceNetUpdate();
	OnRep_Ready();
}
void AC4PlayerState::ChangeBonusSpeed(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0) return;
	const int32 old = BonusSpeed;
	BonusSpeed = FMath::Min(BonusSpeed + Amount, MaxBonusSpeed);
	const int32 newspeed = BonusSpeed - old;

	ForceNetUpdate();
	OnRep_BonusSpeed();
	SetCurrentBonusSpeed(CurrentBonusSpeed + newspeed);
}
void AC4PlayerState::ApplySpeedPenalty(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0) return;
	SetCurrentBonusSpeed(FMath::Max(CurrentBonusSpeed  - Amount, MinBonusSpeed));
	StartRecovery();
}
void AC4PlayerState::SetCurrentBonusSpeed(int32 value)
{
	if (CurrentBonusSpeed == value) return;

	CurrentBonusSpeed = value;
	ForceNetUpdate();
	OnRep_CurrentBonusSpeed();
}
void AC4PlayerState::StartRecovery()
{
	FTimerManager& TM = GetWorldTimerManager();

	if (CurrentBonusSpeed >= BonusSpeed)
	{
		TM.ClearTimer(RecoveryTimerHandle);
		return;
	}
	if (!TM.IsTimerActive(RecoveryTimerHandle))
	{
		TM.SetTimer(RecoveryTimerHandle, this, &ThisClass::TickRecovery, RecoveryInterval, true);
	}
}
void AC4PlayerState::TickRecovery()
{
	const int32 Next = FMath::Min(CurrentBonusSpeed + RecoveryStep, BonusSpeed);
	SetCurrentBonusSpeed(Next);

	if (Next >= BonusSpeed)
	{
		GetWorldTimerManager().ClearTimer(RecoveryTimerHandle);
	}
}
bool AC4PlayerState::AddMine() 
{
	if (!HasAuthority() || MineCount >= MaxMineCount) return false;
	MineCount++;
	ForceNetUpdate();
	OnRep_MineCount();
	return true;
}
bool AC4PlayerState::UseMine() 
{
	if (!HasAuthority() || MineCount <= 0) return false;
	MineCount--;
	ForceNetUpdate();
	OnRep_MineCount();
	return true;
}

void AC4PlayerState::OnRep_Coins() { OnCoinsChanged.Broadcast(Coins); }
void AC4PlayerState::OnRep_CoinScore() { OnScoreChanged.Broadcast(GetTotalScore()); }
void AC4PlayerState::OnRep_StealScore() { OnScoreChanged.Broadcast(GetTotalScore()); }
void AC4PlayerState::OnRep_BonusSpeed() { OnBonusSpeedChanged.Broadcast(BonusSpeed); }
void AC4PlayerState::OnRep_CurrentBonusSpeed() { OnCurrentBonusSpeedChanged.Broadcast(CurrentBonusSpeed); }
void AC4PlayerState::OnRep_MineCount() { OnMineCountChanged.Broadcast(MineCount); }
void AC4PlayerState::OnRep_Ready() { OnReadyChanged.Broadcast(bReady); }

void AC4PlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerNameUpdated.Broadcast();
}

void AC4PlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AC4PlayerState* PS = Cast<AC4PlayerState>(PlayerState))
	{
		PS->Coins = Coins;
		PS->CoinScore = CoinScore;
		PS->StealScore = StealScore;
		PS->BonusSpeed = BonusSpeed;
		PS->CurrentBonusSpeed = BonusSpeed;
		PS->MineCount = MineCount;

		PS->OnRep_Coins();
		PS->OnRep_CoinScore();
		PS->OnRep_BonusSpeed();
		PS->OnRep_CurrentBonusSpeed();
		PS->OnRep_MineCount();
	}
}
void AC4PlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	if (const AC4PlayerState* PS = Cast<AC4PlayerState>(PlayerState))
	{
		Coins = PS->Coins;
		CoinScore = PS->CoinScore;
		StealScore = PS->StealScore;
		BonusSpeed = PS->BonusSpeed;
		CurrentBonusSpeed = PS->BonusSpeed;
		MineCount = PS->MineCount;
	}
}