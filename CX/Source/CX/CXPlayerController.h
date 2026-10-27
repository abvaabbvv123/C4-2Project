// CXPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "System/C4PlayerController.h"
#include "Base/C4Type.h" 
#include "CXPlayerController.generated.h"
#pragma region headerclass
class ACXPawn;
class UCXUI;
class UInputMappingContext;
#pragma endregion


UCLASS(abstract)
class CX_API ACXPlayerController : public AC4PlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float Delta) override;

protected:
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUICreated(UUserWidget* InMainUI) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool bUseSteeringWheelControls = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (EditCondition = "bUseSteeringWheelControls"))
	TObjectPtr<UInputMappingContext> SteeringWheelInputMappingContext;

	UPROPERTY(Replicated)
	TObjectPtr<ACXPawn> VehiclePawn;

	UPROPERTY()
	TObjectPtr<UCXUI> VehicleUI;
};