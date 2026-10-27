// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C4MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class CX_API UC4MainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Connect;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Quit;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextBlock_Info;

private:
	UFUNCTION() void OnConnectClicked();
	UFUNCTION() void OnQuitClicked();

	void ShowInfo(const FText& Message);

	bool bConnecting = false;
};
