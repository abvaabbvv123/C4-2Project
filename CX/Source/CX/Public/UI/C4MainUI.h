// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4MainUI.generated.h"
class UCXUI;
class UC4ChatWidget;
class UTextBlock;
/**
 * 
 */
UCLASS()
class CX_API UC4MainUI : public UUserWidget
{
	GENERATED_BODY()
public:
	UCXUI* GetVehicleUI() const;
	UC4ChatWidget* GetChatWidget() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


	UFUNCTION(BlueprintImplementableEvent, Category = "Info") void OnInfoShown(const FText& Text);

	UPROPERTY(meta = (BindWidget))	TObjectPtr<UCXUI> VehicleUI;
	UPROPERTY(meta = (BindWidget))	TObjectPtr<UC4ChatWidget> ChatWidget;
	UPROPERTY(meta = (BindWidget))	TObjectPtr<UTextBlock> TextBlock_Info;

	UPROPERTY(EditDefaultsOnly, Category = "Info") float InfoDuration = 3.f;

private:
	void HandleInfo(const FText& Text);
	void HideInfo();

	FTimerHandle InfoTimerHandle;
};

