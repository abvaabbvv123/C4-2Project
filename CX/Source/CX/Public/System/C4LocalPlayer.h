// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "C4LocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class CX_API UC4LocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
public:
	virtual FString GetNickname() const override;
};