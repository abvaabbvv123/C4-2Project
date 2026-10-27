// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/C4BaseItem.h"
#include "C4MineItem.generated.h"
class AC4PlayerState;
/**
 * 
 */
UCLASS()
class CX_API AC4MineItem : public AC4BaseItem
{
	GENERATED_BODY()
public:
	AC4MineItem();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual bool ApplyItemEffect(APawn* Player) override;

	UPROPERTY(EditDefaultsOnly, Category = "Item|Mine") float TriggerDelay = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Item|Mine") float PenaltyDuration = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Item|Mine") float MineLifeTime = 60.f;
	UPROPERTY(EditDefaultsOnly, Category = "Item|Mine") int32 CoinLossAmount = 50;

	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Mine")
	void OnTriggeredChanged(bool Triggered);

private:
	void SetMine();
	bool IsSet(const APawn* Player, const AC4PlayerState* PlayerPS) const;

	UFUNCTION()	void OnRep_Triggered();

	//UFUNCTION(Client, Reliable)	void NotifyMineTrigger(const FString& PlayerName);
	UPROPERTY(ReplicatedUsing = OnRep_Triggered)	bool bTrigger = false;
	
	TWeakObjectPtr<AC4PlayerState> SetPlayerState;

	FTimerHandle TriggerTimerHandle;
};
