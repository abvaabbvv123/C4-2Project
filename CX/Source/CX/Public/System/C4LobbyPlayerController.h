// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/C4PlayerController.h"
#include "C4LobbyPlayerController.generated.h"

UCLASS()
class CX_API AC4LobbyPlayerController : public AC4PlayerController
{
	GENERATED_BODY()
public:
	void ToggleReady();
	void RequestNickname(const FString& NewName);

protected:
	virtual void BeginPlay() override;
	virtual void ChangeInputMode(bool bCursorMode) override;

	UFUNCTION(Server, Reliable) void ServerSetReady(bool bNewReady);
	UFUNCTION(Server, Reliable) void ServerSetNickname(const FString& NewName);

	UPROPERTY(EditDefaultsOnly, Category = "Lobby") float ReadyCooldown = 0.3f;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") float NicknameCooldown = 1.f;

private:
	float LastReadyTime = -1000.f;
	float LastNicknameTime = -1000.f;
};