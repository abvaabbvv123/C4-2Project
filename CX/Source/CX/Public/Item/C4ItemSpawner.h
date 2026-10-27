// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C4ItemSpawner.generated.h"

class UBoxComponent;
class AC4BaseItem;
 
UCLASS()
class CX_API AC4ItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AC4ItemSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool TrySpawnItem();
	bool FindSpawnLocation(FVector& OutLocation) const;

	int32 GetMaxCount() const;
	bool bIsMax() const;
	
	void UpdateIncreaseTimer();
	void HandleIncreaseTimer();
	void HandleRespawnTimer();

	UFUNCTION()
	void HandleDestroyed(AActor* Item);

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<UBoxComponent> SpawnArea;
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AC4BaseItem> ItemClass;

	UPROPERTY(EditAnywhere, Category = "Spawner") bool bHasCap = true;
	UPROPERTY(EditAnywhere, Category = "Spawner") bool bFillOnStart = true;
	UPROPERTY(EditAnywhere, Category = "Spawner") int32 MaxItemCount = 5;
	UPROPERTY(EditAnywhere, Category = "Spawner") int32 MaxPlaceCheck = 10;
	UPROPERTY(EditAnywhere, Category = "Spawner") float RespawnDelay = 2.f;
	UPROPERTY(EditAnywhere, Category = "Spawner") float MinRespawnDistance = 500.f; 
	UPROPERTY(EditAnywhere, Category = "Spawner") float GroundOffset = 100.f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AC4BaseItem>> SpawnedItems;

	int32 PendingRespawnCount = 0;

	FVector LastRemovedLocation = FVector::ZeroVector;
	bool bHasLastRemoved = false;

	FTimerHandle IncreaseTimerHandle;

	static constexpr int32 UnlimitedSafetyLimit = 1000;
};
