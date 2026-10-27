// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/C4BaseItem.h"
#include "C4CoinItem.generated.h"

/**
 * 
 */
UCLASS()
class CX_API AC4CoinItem : public AC4BaseItem
{
	GENERATED_BODY()
public:
	AC4CoinItem();
protected:
	virtual bool ApplyItemEffect(APawn* CollectorPawn) override;

	UPROPERTY(EditAnywhere, Category = "Item|Coin")
	int32 CoinAmount = 10;
};
