// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/C4ItemSpawner.h"
#include "Item/C4BaseItem.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"


AC4ItemSpawner::AC4ItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	bNetLoadOnClient = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetBoxExtent(FVector(1000.f, 1000.f, 200.f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnArea->SetGenerateOverlapEvents(false);
	RootComponent = SpawnArea;

}
void AC4ItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority()) return;

	const int32 InitCount = (bHasCap && bFillOnStart) ? GetMaxCount() : 1;
	for (int32 i = 0; i < InitCount; i++)
	{
		TrySpawnItem();
	}
	UpdateIncreaseTimer();
}
void AC4ItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	for (AC4BaseItem* s : SpawnedItems)
	{
		if (IsValid(s))
		{
			s->OnDestroyed.RemoveDynamic(this, &ThisClass::HandleDestroyed);
		}
	}
	SpawnedItems.Empty();
	PendingRespawnCount = 0;
	Super::EndPlay(EndPlayReason);
}
// for server
// ----
int32 AC4ItemSpawner::GetMaxCount() const
{
	return bHasCap ? MaxItemCount : UnlimitedSafetyLimit;
}
bool AC4ItemSpawner::bIsMax() const
{
	return SpawnedItems.Num() + PendingRespawnCount < GetMaxCount();
}
void AC4ItemSpawner::UpdateIncreaseTimer()
{
	FTimerManager& TM = GetWorldTimerManager();
	const bool bActive = TM.IsTimerActive(IncreaseTimerHandle);

	if (bIsMax() && !bActive)
	{
		TM.SetTimer(IncreaseTimerHandle, this, &ThisClass::HandleIncreaseTimer, RespawnDelay, true);
	}
	else if (!bIsMax() && bActive)
	{
		TM.ClearTimer(IncreaseTimerHandle);
	}
}
void AC4ItemSpawner::HandleIncreaseTimer()
{
	if (bIsMax()) TrySpawnItem();
	UpdateIncreaseTimer();
}
void AC4ItemSpawner::HandleRespawnTimer()
{
	PendingRespawnCount = FMath::Max(PendingRespawnCount - 1, 0);

	if (SpawnedItems.Num() < GetMaxCount() && !TrySpawnItem())
	{
		PendingRespawnCount++;
		FTimerHandle Retry;
		GetWorldTimerManager().SetTimer(Retry, this, &ThisClass::HandleRespawnTimer, RespawnDelay, false);
	}
	UpdateIncreaseTimer();
}
void AC4ItemSpawner::HandleDestroyed(AActor* DestroyedActor)
{
	AC4BaseItem* Item = Cast<AC4BaseItem>(DestroyedActor);
	if (!Item || SpawnedItems.RemoveSwap(Item) == 0) return;

	LastRemovedLocation = Item->GetActorLocation();
	bHasLastRemoved = true;

	const UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)return; 

	PendingRespawnCount++;
	FTimerHandle RespawnHandle;
	GetWorldTimerManager().SetTimer(RespawnHandle, this, &ThisClass::HandleRespawnTimer, RespawnDelay, false);
}
bool AC4ItemSpawner::TrySpawnItem()
{
	if (!ItemClass || SpawnedItems.Num() >= GetMaxCount()) return false;

	FVector SpawnLocation;
	if (!FindSpawnLocation(SpawnLocation)) return false;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator SpawnRotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
	AC4BaseItem* Item = GetWorld()->SpawnActor<AC4BaseItem>(ItemClass, SpawnLocation, SpawnRotation, Params);
	if (!Item) return false;

	SpawnedItems.Add(Item);
	Item->OnDestroyed.AddDynamic(this, &ThisClass::HandleDestroyed);
	return true;
}

bool AC4ItemSpawner::FindSpawnLocation(FVector& OutLocation) const
{
	const FVector Location = SpawnArea->GetUnscaledBoxExtent();
	const FTransform& Area = SpawnArea->GetComponentTransform();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ItemSpawnerGroundTrace), false, this);
	const FCollisionObjectQueryParams ObjectParams(ECC_WorldStatic);
	const float MinDistSq = FMath::Square(MinRespawnDistance);

	bool bHasFallback = false;

	for (int32 i = 0; i < MaxPlaceCheck; ++i)
	{
		const float X = FMath::FRandRange(-Location.X, Location.X);
		const float Y = FMath::FRandRange(-Location.Y, Location.Y);

		const FVector Start = Area.TransformPosition(FVector(X, Y, Location.Z));
		const FVector End = Area.TransformPosition(FVector(X, Y, -Location.Z));

		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjectParams, QueryParams))	continue;

		const FVector Candidate = Hit.ImpactPoint + FVector(0.f, 0.f, GroundOffset);
		OutLocation = Candidate;
		bHasFallback = true;

		bool bTooClose = bHasLastRemoved && FVector::DistSquared(Candidate, LastRemovedLocation) < MinDistSq;

		for (int32 j = 0; !bTooClose && j < SpawnedItems.Num(); ++j)
		{
			const AC4BaseItem* Other = SpawnedItems[j];
			bTooClose = IsValid(Other) && FVector::DistSquared(Candidate, Other->GetActorLocation()) < MinDistSq;
		}

		if (!bTooClose) return true;
	}
	return bHasFallback;
}