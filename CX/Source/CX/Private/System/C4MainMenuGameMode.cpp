// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4MainMenuGameMode.h"
#include "System/C4MainMenuPlayerController.h"

AC4MainMenuGameMode::AC4MainMenuGameMode()
{
	PlayerControllerClass = AC4MainMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}