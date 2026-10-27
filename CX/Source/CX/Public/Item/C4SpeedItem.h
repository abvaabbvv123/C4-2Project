// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/C4BaseItem.h"
#include "C4SpeedItem.generated.h"

/**
 * 
 */
UCLASS()
class CX_API AC4SpeedItem : public AC4BaseItem
{
	GENERATED_BODY()
public:
	AC4SpeedItem();
protected:
	virtual bool ApplyItemEffect(APawn* Player) override;
	UPROPERTY(EditAnywhere, Category = "Item|Speed")
	int32 BonusSpeed = 5;

};
