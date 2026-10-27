// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Base/C4Type.h"   // EC4MatchState (generated.h include보다 위에)
#include "CXPawn.generated.h"
class AGameStateBase;
class AC4GameState;
class AC4MineItem;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;
class AC4PlayerState;
class USphereComponent;
class UNiagaraSystem;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class ACXPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ResetVehicleAction;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

public:
	ACXPawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface

	virtual void Tick(float Delta) override;

	// End Actor interface

protected:

	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

#pragma region BonusSpeed
public: 	
	void ApplyBonusSpeed(int32 BonusPercent);
protected:
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;

private:
	void BindPS();
	void UnbindPS();

	float BaseMaxTorque = 0.f;

	TWeakObjectPtr<AC4PlayerState> BoundPlayerState;
#pragma endregion 
#pragma region NetSetting
public:
	float GetItemRelevancyRadius() const;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	TObjectPtr<USphereComponent> ItemRelevancySphere;
#pragma endregion 
#pragma region Attack
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,AController* EventInstigator, AActor* DamageCauser) override;
	bool IsWheelHit(const FHitResult& Hit) const;
	bool IsWheel(FName BoneName) const; 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = Input)
	UInputAction* AttackAction;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") float AttackCooldown = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") float AttackRange = 600.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") float AttackTraceRadius = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") float WheelHitRadius = 70.f;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") FName AttackSocketName = TEXT("Attack Point");
	UPROPERTY(EditDefaultsOnly, Category = "Attack") TArray<FName> WheelBoneNames;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") FVector AttackTraceLocalOffset = FVector(250.f, 0.f, 60.f);
	UPROPERTY(EditDefaultsOnly, Category = "Attack") int32 CoinTake = 20;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") int32 SpeedPenalty = 95;
	UPROPERTY(EditDefaultsOnly, Category = "Attack") TObjectPtr<UNiagaraSystem> AttackHitVFX;
	
	UFUNCTION(Server, Reliable) void ServerAttackCheck();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastAttackFX(const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd, bool bHitVehicle, bool bHitWheel);
	void Attack();
private:
	FVector GetAttackTraceStart() const;
	float LocalLastAttackTime = -1000.f;
	float ServerLastAttackTime = -1000.f;

#pragma endregion
#pragma region Crash
	public:
		virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
		UPROPERTY(EditDefaultsOnly, Category = "Crash") float CrashCoolDown = 1.f;
		UPROPERTY(EditDefaultsOnly, Category = "Crash") float MinCrashSpeed = 40.f;
		UPROPERTY(EditDefaultsOnly, Category = "Crash") float MaxWallNormalZ = 0.5f;
		UPROPERTY(EditDefaultsOnly, Category = "Crash") float MinImpactHeight = 30.f;
		UPROPERTY(EditDefaultsOnly, Category = "Crash") float CrashLossCoin = 5.f;
		UPROPERTY(EditDefaultsOnly, Category = "Crash")	FName CrashTag = TEXT("Wall");
	private:
		bool IsValidWallCrash(const FHitResult& Hit, const AActor* Other, const UPrimitiveComponent* OtherComp,	float& OutImpactSpeedKmh) const;
		FVector PreImpactVelocity = FVector::ZeroVector;
		float LastCrashTime = -1000.f;
#pragma endregion
#pragma region Mine
public:

	void ApplyPenalty(float Duration);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MineAction;
	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	TSubclassOf<AC4MineItem> MineClass;


	UPROPERTY(EditDefaultsOnly, Category = "Mine") float MineBackOffset = 350.f;
	UPROPERTY(EditDefaultsOnly, Category = "Mine") float MineCooldown = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Mine") float MineGroundOffset = 5.f;

	
	void PlaceMine();
	UFUNCTION(Server, Reliable)
	void PlaceMinebyServer();

	UFUNCTION()
	void OnRep_Penalty();

	UPROPERTY(ReplicatedUsing = OnRep_Penalty)
	bool bInPenalty = false;

private:
	void ReleasePenalty();
	bool FindMineSpawnLocation(FVector& OutLocation) const;

	FTimerHandle ThrottleLockTimerHandle;

	float LocalLastMineTime = -1000.f;
	float ServerLastMineTime = -1000.f;
#pragma endregion
#pragma region MatchInput
protected:
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;

	bool IsMatchPlaying() const;

private:
	void BindGameState(AGameStateBase* NewGameState);
	void UnbindGameState();
	void HandleMatchStateChanged(EC4MatchState NewState);
	void ApplyMatchInputLock();

	bool bMatchInputAllowed = false;
	FDelegateHandle GameStateSetHandle;
	TWeakObjectPtr<AC4GameState> BoundGameState;
#pragma endregion
};
