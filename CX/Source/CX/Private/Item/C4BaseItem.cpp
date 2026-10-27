// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/C4BaseItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CXPawn.h"
#include "System/C4GameState.h"
#include "GameFramework/PlayerController.h"


// Sets default values
AC4BaseItem::AC4BaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionComp->InitSphereRadius(100.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp ->SetupAttachment(CollisionComp);
	MeshComp ->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetGenerateOverlapEvents(false);

	RotatingComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotating Movement"));
	RotatingComp->SetUpdatedComponent(MeshComp);
	RotatingComp->RotationRate = FRotator(0.f, RotateSpeed, 0.f);
}

// Called when the game starts or when spawned
void AC4BaseItem::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
	}
	RotatingComp->RotationRate = FRotator(0.f, RotateSpeed, 0.f);
	if (!bRotating || GetNetMode() == NM_DedicatedServer)
	{
		RotatingComp->Deactivate();
	}
}

// Called every frame
//void AC4BaseItem::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//
// }
bool AC4BaseItem::ApplyItemEffect(APawn* Player)
{
	return false;
}

// for server

void AC4BaseItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected)	return; //prevent collect collected

	APawn* CollectorPawn = Cast<APawn>(OtherActor);
	if (!IsValid(CollectorPawn) || !ApplyItemEffect(CollectorPawn)) return;

	bCollected = true; 
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	RotatingComp->Deactivate();
	ForceNetUpdate();
	MulticastPlayPickupEffects();
	SetLifeSpan(DestroyDelay); // if detroy instant -> might dont call multicast
}
//for server and all client 
void AC4BaseItem::MulticastPlayPickupEffects_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer) return;
	PlayPickupEffects();
}
void AC4BaseItem::PlayPickupEffects()
{
	const FVector EffectLocation = GetActorLocation() + VFXOffset;

	if (PickupVFX)
	{
		UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupVFX, EffectLocation, GetActorRotation(), VFXScale, true, true, ENCPoolMethod::AutoRelease);

		if (VFX && VFXDuration > 0.f)
		{
			TWeakObjectPtr<UNiagaraComponent> WeakVFX = VFX;
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([WeakVFX]()
				{
					if (WeakVFX.IsValid())
					{
						WeakVFX->Deactivate();
					}
				}), VFXDuration, false);
		}
	}

	if (PickupSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSFX, EffectLocation,SFXVolume, SFXPitch, 0.f, SFXAttenuation);
	}
}
bool AC4BaseItem::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (IsOwnedBy(RealViewer) || IsOwnedBy(ViewTarget))	return true;

	const ACXPawn* Pawn = Cast<ACXPawn>(ViewTarget);
	if (!Pawn)
	{
		if (const APlayerController* PC = Cast<APlayerController>(RealViewer))
		{
			Pawn = Cast<ACXPawn>(PC->GetPawn());
		}
	}

	if (!Pawn)
	{
		return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	}

	const float Radius = Pawn->GetItemRelevancyRadius();
	return FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation()) <= FMath::Square(Radius);
}