// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C4BaseItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class URotatingMovementComponent;
class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;

UCLASS()
class CX_API AC4BaseItem : public AActor
{
	GENERATED_BODY()
	
public:
	AC4BaseItem();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

protected:
	virtual void BeginPlay() override;
	virtual bool ApplyItemEffect(APawn* Player); // overlap event

	UPROPERTY(VisibleAnywhere, Category = "Item|Component")
	TObjectPtr<USphereComponent> CollisionComp;
	UPROPERTY(VisibleAnywhere, Category = "Item|Component")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Item|Component")
	TObjectPtr<URotatingMovementComponent> RotatingComp;

	//setting 
	// ----
	UPROPERTY(EditDefaultsOnly, Category = "Item|Settings")
	bool bRotating = false;
	UPROPERTY(EditDefaultsOnly, Category = "Item|Settings")
	float RotateSpeed = 180.f;
	// ----
	//VFX
	// ----
	UPROPERTY(EditAnywhere, Category = "Item|VFX")
	TObjectPtr<UNiagaraSystem> PickupVFX;
	UPROPERTY(EditAnywhere, Category = "Item|VFX")
	FVector VFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, Category = "Item|VFX")
	FVector VFXOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Item|VFX")
	float VFXDuration = 0.f;
	// ----
	// SFX
	// ---
	UPROPERTY(EditAnywhere, Category = "Item|SFX")
	TObjectPtr<USoundBase> PickupSFX;
	UPROPERTY(EditAnywhere, Category = "Item|SFX")
	float SFXVolume = 1.f;
	UPROPERTY(EditAnywhere, Category = "Item|SFX")
	float SFXPitch = 1.f;
	UPROPERTY(EditAnywhere, Category = "Item|SFX")
	TObjectPtr<USoundAttenuation> SFXAttenuation;
	// -----
	
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	float DestroyDelay = 1.f;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Unreliable) // server -> all other clients
	void MulticastPlayPickupEffects();

	void PlayPickupEffects();

	bool bCollected = false;
};
	