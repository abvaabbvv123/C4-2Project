// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Engine/DamageEvents.h"  
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "System/C4PlayerState.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "CXPlayerController.h"  
#include "Item/C4MineItem.h"
#include "Net/UnrealNetwork.h"
#include "System/C4GameState.h"
#include "GameFramework/GameStateBase.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	bAlwaysRelevant = true;

	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetNotifyRigidBodyCollision(true);
	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	//net setting
	ItemRelevancySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemRelevancySphere"));
	ItemRelevancySphere->SetupAttachment(GetMesh());
	ItemRelevancySphere->InitSphereRadius(50000.f);
	ItemRelevancySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemRelevancySphere->SetGenerateOverlapEvents(false);
	ItemRelevancySphere->SetCanEverAffectNavigation(false);
	ItemRelevancySphere->SetHiddenInGame(true);
}

void ACXPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		// aatackk
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::Attack);
		}
		//set mine
		if (MineAction)
		{
			EnhancedInputComponent->BindAction(MineAction, ETriggerEvent::Started, this, &ACXPawn::PlaceMine);
		}
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);
	if (HasAuthority())
	{
		PreImpactVelocity = GetVelocity();
	}
	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	ChaosVehicleMovement->SetSteeringInput(bMatchInputAllowed ? Value.Get<float>() : 0.f);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	if (!bMatchInputAllowed || bInPenalty)
	{
		ChaosVehicleMovement->SetThrottleInput(0.f);
		return;
	}
	ChaosVehicleMovement->SetThrottleInput(Value.Get<float>());
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
		}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	if (!bMatchInputAllowed) return;
	ChaosVehicleMovement->SetHandbrakeInput(true);
	BrakeLights(true);
}


void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	if (!bMatchInputAllowed) return;
	ChaosVehicleMovement->SetHandbrakeInput(false);
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (!bMatchInputAllowed) return;
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}
#pragma region BonusSpeed
void ACXPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BaseMaxTorque = GetChaosVehicleMovement()->EngineSetup.MaxTorque;
}
void ACXPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindGameState();
	UnbindPS();
	Super::EndPlay(EndPlayReason);
}
void ACXPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	BindPS();
}
void ACXPawn::UnPossessed()
{
	UnbindPS();
	Super::UnPossessed();
}
void ACXPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindPS();
}

void ACXPawn::BindPS()
{
	AC4PlayerState* C4PS = GetPlayerState<AC4PlayerState>();
	if (BoundPlayerState.Get() == C4PS) return;

	UnbindPS();
	if (!IsValid(C4PS)) return;

	BoundPlayerState = C4PS;
	C4PS->OnCurrentBonusSpeedChanged.AddUObject(this, &ThisClass::ApplyBonusSpeed);
	ApplyBonusSpeed(C4PS->GetCurrentBonusSpeed());
}
void ACXPawn::UnbindPS()
{
	if (AC4PlayerState* C4PS = BoundPlayerState.Get())
	{
		C4PS->OnCurrentBonusSpeedChanged.RemoveAll(this);
	}
	BoundPlayerState.Reset();
}
void ACXPawn::ApplyBonusSpeed(int32 BonusPercent)
{
	if (BaseMaxTorque <= 0.f) return;

	const float NewTorque = BaseMaxTorque * (100 + BonusPercent) / 100.f;
	GetChaosVehicleMovement()->SetMaxEngineTorque(NewTorque);
}
#pragma endregion
#pragma region NetSetting
float ACXPawn::GetItemRelevancyRadius() const
{
	return ItemRelevancySphere->GetScaledSphereRadius();
}
#pragma endregion
#pragma region Attack
void ACXPawn::Attack()
{
	if (!bMatchInputAllowed) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LocalLastAttackTime < AttackCooldown) return;   //prevent auto press
	LocalLastAttackTime = Now;

	ServerAttackCheck();
}
//server attack
void ACXPawn::ServerAttackCheck_Implementation()
{
	if (!IsMatchPlaying()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - ServerLastAttackTime < AttackCooldown) return;
	ServerLastAttackTime = Now;

	const FVector Forward = GetActorForwardVector();
	const FVector TraceStart = GetAttackTraceStart();
	const FVector TraceEnd = TraceStart + Forward * AttackRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehicleAttackTrace), false, this);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Vehicle);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity,
		ObjectParams, FCollisionShape::MakeSphere(AttackTraceRadius), QueryParams);

	ACXPawn* Taken = bHit ? Cast<ACXPawn>(Hit.GetActor()) : nullptr;
	bool bHitWheel = false;
	if (Taken)
	{
		bHitWheel = Taken->IsWheelHit(Hit);
		UGameplayStatics::ApplyPointDamage(Taken, 1.f, Forward, Hit, GetController(), this, UDamageType::StaticClass());
	}
	MulticastAttackFX(TraceStart, bHit ? FVector(Hit.ImpactPoint) : TraceEnd, Taken != nullptr, bHitWheel);
}
float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.f;
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f) return 0.f;

	AC4PlayerState* HItPlayerPS = GetPlayerState<AC4PlayerState>();
	if (!HItPlayerPS) return 0.f;

	//ssteal
	AC4PlayerState* AttackerPS = EventInstigator ? EventInstigator->GetPlayerState<AC4PlayerState>() : nullptr;
	if (AttackerPS && AttackerPS != HItPlayerPS)
	{
		AttackerPS->StealCoins(HItPlayerPS, CoinTake);
	}
	// Hit wheel -> speed penalty
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
		if (IsWheelHit(PointEvent.HitInfo))
		{
			HItPlayerPS->ApplySpeedPenalty(SpeedPenalty);
		}
	}
	return ActualDamage;
}
bool ACXPawn::IsWheel(FName BoneName) const
{
	if (BoneName.IsNone()) return false;
	if (WheelBoneNames.Num() > 0)
	{
		return WheelBoneNames.Contains(BoneName);
	}
	return BoneName.ToString().Contains(TEXT("wheel"), ESearchCase::IgnoreCase);
}
bool ACXPawn::IsWheelHit(const FHitResult& Hit) const
{
	if (IsWheel(Hit.BoneName)) return true;

	const float RadiusSq = FMath::Square(WheelHitRadius);
	for (const FChaosWheelSetup& Setup : ChaosVehicleMovement->WheelSetups)
	{
		if (Setup.BoneName.IsNone()) continue;
		if (FVector::DistSquared(GetMesh()->GetBoneLocation(Setup.BoneName), Hit.ImpactPoint) <= RadiusSq)
		{
			return true;
		}
	}
	return false;
}
FVector ACXPawn::GetAttackTraceStart() const
{
	if (!AttackSocketName.IsNone() && GetMesh()->DoesSocketExist(AttackSocketName))
	{
		return GetMesh()->GetSocketLocation(AttackSocketName);
	}
	return GetActorTransform().TransformPosition(AttackTraceLocalOffset);
}
void ACXPawn::MulticastAttackFX_Implementation(const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& TraceEnd, bool bHitVehicle, bool bHitWheel)
{
	if (GetNetMode() == NM_DedicatedServer) return;

#if ENABLE_DRAW_DEBUG
	const FColor LineColor = bHitWheel ? FColor::Yellow : (bHitVehicle ? FColor::Green : FColor::Red);
	DrawDebugSphere(GetWorld(), TraceEnd, AttackTraceRadius, 12, LineColor, false, 3.f);
#endif

	if (bHitVehicle && AttackHitVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, AttackHitVFX, TraceEnd, FRotator::ZeroRotator,	FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}
}
#pragma endregion
#pragma region Crash
void ACXPawn::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	if (!HasAuthority()) return;
	if (!IsMatchPlaying()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastCrashTime < CrashCoolDown) return; // still cool down

	float ImpactSpeed = 0.f;
	if (!IsValidWallCrash(Hit, Other, OtherComp, ImpactSpeed)) return; //not wall
	LastCrashTime = Now; 

	AC4PlayerState* C4PS = GetPlayerState<AC4PlayerState>();
	if (!C4PS)	return;
	const int32 LossCoin = FMath::FloorToInt(ImpactSpeed / CrashLossCoin);
	const int32 Lost = C4PS->LoseCoins(LossCoin);
	// for debug, need to delete
	if (Lost > 0)
	{
		if (ACXPlayerController* CXPC = Cast<ACXPlayerController>(GetController()))
		{
			CXPC->ClientShowInfo(FC4Info(EC4InfoType::CrashCoinLost, Lost));
		}
	}
}



//check wall valid 

bool ACXPawn::IsValidWallCrash(const FHitResult& Hit, const AActor* Other, const UPrimitiveComponent* OtherComp, float& OutImpactSpeedKmh) const
{
	if (!Other || !OtherComp || Cast<ACXPawn>(Other)) return false;
	if (!Other->ActorHasTag(CrashTag) && !OtherComp->ComponentHasTag(CrashTag)) return false;

	const FVector WallNormal = Hit.ImpactNormal;
	if (FMath::Abs(WallNormal.Z) > MaxWallNormalZ) return false;

	if (MinImpactHeight > 0.f)
	{
		const FBoxSphereBounds& MeshBounds = GetMesh()->Bounds;
		const float VehicleBottomZ = MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z;
		if (Hit.ImpactPoint.Z - VehicleBottomZ < MinImpactHeight) return false;
	}
	const FVector CurrentVelocity = GetVelocity();
	const FVector Velocity = PreImpactVelocity.SizeSquared() > CurrentVelocity.SizeSquared() ? PreImpactVelocity : CurrentVelocity;
	OutImpactSpeedKmh = FMath::Abs(FVector::DotProduct(Velocity, WallNormal)) * 0.036f; // cm/s ¡æ km/h

	return OutImpactSpeedKmh >= MinCrashSpeed;
}

#pragma endregion 
#pragma region Mine
void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ThisClass, bInPenalty, COND_OwnerOnly);
}
void ACXPawn::PlaceMine()
{
	if (!bMatchInputAllowed) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LocalLastMineTime < MineCooldown) return;
	
	const AC4PlayerState* C4PS = GetPlayerState<AC4PlayerState>();
	if (!C4PS || C4PS->GetMineCount() <= 0) return;

	LocalLastMineTime = Now;
	PlaceMinebyServer();
}
void ACXPawn::PlaceMinebyServer_Implementation()
{
	if (!IsMatchPlaying()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - ServerLastMineTime < MineCooldown || !MineClass)	return;

	AC4PlayerState* C4PS = GetPlayerState<AC4PlayerState>();
	if (!C4PS || C4PS->GetMineCount() <= 0)	return; 
	

	FVector SpawnLocation;
	if (!FindMineSpawnLocation(SpawnLocation)) return;

	FActorSpawnParameters Params;
	Params.Owner = this;       
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FRotator SpawnRotation(0.f, GetActorRotation().Yaw, 0.f);
	AC4MineItem* Mine = GetWorld()->SpawnActor<AC4MineItem>(MineClass, SpawnLocation, SpawnRotation, Params);
	if (!Mine) return;

	C4PS->UseMine();
	ServerLastMineTime = Now;
}

bool ACXPawn::FindMineSpawnLocation(FVector& OutLocation) const
{
	const FVector Point = GetActorLocation() - GetActorForwardVector() * MineBackOffset;
	const FVector Start = Point + FVector(0.f, 0.f, 200.f);
	const FVector End = Point - FVector(0.f, 0.f, 400.f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MineGroundTrace), false, this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams)) return false;

	OutLocation = Hit.ImpactPoint + FVector(0.f, 0.f, MineGroundOffset);
	return true;
}
void ACXPawn::ApplyPenalty(float Duration)
{
	if (!HasAuthority()) return;

	bInPenalty = true;
	ChaosVehicleMovement->SetThrottleInput(0.f);
	ForceNetUpdate();
	GetWorldTimerManager().SetTimer(ThrottleLockTimerHandle, this, &ThisClass::ReleasePenalty, Duration, false);
}

void ACXPawn::ReleasePenalty()
{
	bInPenalty = false;
	ForceNetUpdate();
}

void ACXPawn::OnRep_Penalty()
{
	if (bInPenalty)
	{
		ChaosVehicleMovement->SetThrottleInput(0.f);
	}
}
#pragma endregion
#pragma region MatchInput
void ACXPawn::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() == NM_DedicatedServer) return;

	UWorld* World = GetWorld();
	if (AGameStateBase* GS = World->GetGameState())
	{
		BindGameState(GS);
	}
	else
	{
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &ThisClass::BindGameState);
	}
}

void ACXPawn::BindGameState(AGameStateBase* NewGameState)
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	AC4GameState* GS = Cast<AC4GameState>(NewGameState);
	if (!GS)
	{
		bMatchInputAllowed = true;
		ApplyMatchInputLock();
		return;
	}

	BoundGameState = GS;
	GS->OnMatchStateChanged.AddUObject(this, &ThisClass::HandleMatchStateChanged);
	HandleMatchStateChanged(GS->GetMatchState());
}

void ACXPawn::UnbindGameState()
{
	if (UWorld* World = GetWorld())
	{
		World->GameStateSetEvent.Remove(GameStateSetHandle);
	}
	GameStateSetHandle.Reset();

	if (AC4GameState* GS = BoundGameState.Get())
	{
		GS->OnMatchStateChanged.RemoveAll(this);
	}
	BoundGameState.Reset();
}

void ACXPawn::HandleMatchStateChanged(EC4MatchState NewState)
{
	bMatchInputAllowed = (NewState == EC4MatchState::Playing);
	ApplyMatchInputLock();
}
void ACXPawn::ApplyMatchInputLock()
{
	if (!IsLocallyControlled()) return;

	if (bMatchInputAllowed)
	{
		ChaosVehicleMovement->SetHandbrakeInput(false);
	}
	else
	{
		ChaosVehicleMovement->SetThrottleInput(0.f);
		ChaosVehicleMovement->SetSteeringInput(0.f);
		ChaosVehicleMovement->SetBrakeInput(0.f);
		ChaosVehicleMovement->SetHandbrakeInput(true); 
	}
	BrakeLights(!bMatchInputAllowed);
}
void ACXPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	ApplyMatchInputLock();
}
bool ACXPawn::IsMatchPlaying() const
{
	const AGameStateBase* GSB = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GSB) return false;
	const AC4GameState* GS = Cast<AC4GameState>(GSB);
	return !GS || GS->IsPlaying();
}
#pragma endregion
#undef LOCTEXT_NAMESPACE