// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/C4MinePickupItem.h"
#include "System/C4PlayerState.h"
#include "GameFramework/Pawn.h"

AC4MinePickupItem::AC4MinePickupItem()
{
	SetNetUpdateFrequency(1.f);
}
bool AC4MinePickupItem::ApplyItemEffect(APawn* Player)
{
	AC4PlayerState* C4PS = Player->GetPlayerState<AC4PlayerState>();
	if (!IsValid(C4PS)) return false;

	return C4PS->AddMine();
}
