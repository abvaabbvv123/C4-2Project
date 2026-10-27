// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/C4SpeedItem.h"
#include "System/C4PlayerState.h"
#include "GameFramework/Pawn.h"

AC4SpeedItem::AC4SpeedItem()
{
	SetNetUpdateFrequency(1.f);
}
bool AC4SpeedItem::ApplyItemEffect(APawn* Player)
{
	AC4PlayerState* PS = Player->GetPlayerState<AC4PlayerState>();
	if (!IsValid(PS)) return false;
	PS->ChangeBonusSpeed(BonusSpeed);
	return true;
}

