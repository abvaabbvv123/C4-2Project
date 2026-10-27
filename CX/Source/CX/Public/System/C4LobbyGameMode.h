// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/C4GameModeBase.h"
#include "C4LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CX_API AC4LobbyGameMode : public AC4GameModeBase
{
	GENERATED_BODY()

public:
	AC4LobbyGameMode();
	void EvaluateCountdown(const APlayerState* Exclude = nullptr);

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

private:
	
	bool AreAllReady(const APlayerState* Exclude, int32& OutPlayerCount) const;
	void StartCountdown();
	void CancelCountdown();
	void TickCountdown();
	void TravelToGame();
	
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") int32 RequiredPlayers = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") int32 CountdownDuration = 5;
	UPROPERTY(EditDefaultsOnly, Category = "Lobby") TSoftObjectPtr<UWorld> InGameMap;

	FTimerHandle CountdownTimerHandle;
	bool bCountingDown = false;
	bool bTravelling = false;
};