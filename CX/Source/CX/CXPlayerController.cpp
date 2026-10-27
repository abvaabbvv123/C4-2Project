// CXPlayerController.cpp


#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "UI/C4MainUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"


void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}
void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, VehiclePawn);
}
void ACXPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);
	if (!IsLocalController() || !IsValid(VehiclePawn) || !IsValid(VehicleUI)) return;

	UChaosWheeledVehicleMovementComponent* Move = VehiclePawn->GetChaosVehicleMovement();
	VehicleUI->UpdateSpeed(Move->GetForwardSpeed());
	VehicleUI->UpdateGear(Move->GetCurrentGear());
}
void ACXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	VehiclePawn = Cast<ACXPawn>(InPawn);
}

void ACXPlayerController::OnUICreated(UUserWidget* InMainUI)
{
	if (UC4MainUI* C4MainUI = Cast<UC4MainUI>(InMainUI))
	{
		VehicleUI = C4MainUI->GetVehicleUI();
	}
}
