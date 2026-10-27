// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/C4BaseItem.h"
#include "C4MinePickupItem.generated.h"

/**
 * 
 */
UCLASS()
class CX_API AC4MinePickupItem : public AC4BaseItem
{
	GENERATED_BODY()
	
public:
	AC4MinePickupItem();
protected:
	virtual bool ApplyItemEffect(APawn* Player) override;
};
