// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/C4CoinItem.h"
#include "System/C4PlayerState.h"
#include "GameFramework/Pawn.h"
AC4CoinItem::AC4CoinItem()
{
	SetNetUpdateFrequency(1.f);
}
bool AC4CoinItem::ApplyItemEffect(APawn* Player)
{
	AC4PlayerState* PS = Player->GetPlayerState<AC4PlayerState>();
	if (!IsValid(PS))
	{
		return false;
	}
	PS->AddCoins(CoinAmount);
	return true;
}
