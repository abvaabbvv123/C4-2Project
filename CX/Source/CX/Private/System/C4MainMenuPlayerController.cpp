// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AC4MainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController() || !MainMenuClass) return;

	MainMenu = CreateWidget<UUserWidget>(this, MainMenuClass);
	if (!MainMenu) return;
	MainMenu->AddToViewport();

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(MainMenu->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	SetShowMouseCursor(true);
}
