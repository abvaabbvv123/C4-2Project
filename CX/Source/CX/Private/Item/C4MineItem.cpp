// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/C4MineItem.h"
#include "CXPawn.h"
#include "CXPlayerController.h"
#include "System/C4PlayerState.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "System/C4PlayerController.h" 
AC4MineItem::AC4MineItem()
{
	SetNetUpdateFrequency(1.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RotatingComp->RotationRate = FRotator::ZeroRotator;
	RotatingComp->SetAutoActivate(false);
}
void AC4MineItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bTrigger);
}
void AC4MineItem::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority()) return;

	if (const APawn* Player = Cast<APawn>(GetOwner()))
	{
		SetPlayerState = Player->GetPlayerState<AC4PlayerState>();
	}
	if (MineLifeTime > 0.f)
	{
		SetLifeSpan(MineLifeTime);
	}
	if (TriggerDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(TriggerTimerHandle, this, &ThisClass::SetMine, TriggerDelay, false);
	}
	else
	{
		SetMine();
	}
}
void AC4MineItem::SetMine()
{
	bTrigger = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForceNetUpdate();
	OnRep_Triggered();
}
void AC4MineItem::OnRep_Triggered()
{
	OnTriggeredChanged(bTrigger);
}
bool AC4MineItem::ApplyItemEffect(APawn* Player)
{
	if (!bTrigger) return false; 
	ACXPawn* HitPlayer = Cast<ACXPawn>(Player);
	if (!HitPlayer) return false;

	HitPlayer->ApplyPenalty(PenaltyDuration);

	AC4PlayerState* PlayerPS = HitPlayer->GetPlayerState<AC4PlayerState>();
	if (PlayerPS && !IsSet(HitPlayer, PlayerPS))
	{
		const int32 Lost = PlayerPS->LoseCoins(CoinLossAmount);
		if (Lost > 0)
		{
			if (AC4PlayerController* PC = Cast<AC4PlayerController>(HitPlayer->GetController()))
			{
				PC->ClientShowInfo(FC4Info(EC4InfoType::MineCoinLost, Lost));
			}
		}
	}
	return true;
}
bool AC4MineItem::IsSet(const APawn* Player, const AC4PlayerState* PlayerPS) const
{
	if (Player && Player == GetOwner()) return true;
	return PlayerPS && PlayerPS == SetPlayerState.Get();
}
//void AC4MineItem::NotifyMineTrigger_Implementation(const FString& PlayerName) {}