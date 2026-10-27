// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "System/C4GameModeBase.h"
#include "Base/C4Type.h"
#include "CXGameMode.generated.h"

class APlayerStart;

UCLASS(MinimalAPI)
class ACXGameMode : public AC4GameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual bool CanAcceptNewPlayer(FString& OutReason) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

	// Match flow
	void TryStartCountdown(); // countdown while ready
	void OnArrivalTimeout(); 
	void BeginCountdown(); // waiting countdown
	// after start
	void EnterPhase(EC4MatchState NewState, int32 Duration);// Phase -> 
	void TickMatchTimer(); // match timer
	void StartMatch();
	void EndMatch(const APlayerState* Leaver = nullptr);

	void ReturnToLobby();
	int32 CountPlayers(const APlayerState* Exclude) const;
	int32 CountArrived(); 

	TArray<TWeakObjectPtr<APlayerController>> ArrivedPlayers;

	int32 ExpectedPlayers = 0;
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalf = FVector(250.0, 120.0, 80.0);
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

	UPROPERTY(EditDefaultsOnly, Category = "Match")	int32 MinPlayersToStart = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Match")	int32 WaitingDuration = 5;
	UPROPERTY(EditDefaultsOnly, Category = "Match")	int32 MatchDuration = 300;
	UPROPERTY(EditDefaultsOnly, Category = "Match")	int32 ResultDuration = 10;
	UPROPERTY(EditDefaultsOnly, Category = "Match")	float ArrivalTimeout = 20.f;
	UPROPERTY(EditDefaultsOnly, Category = "Match")	TSoftObjectPtr<UWorld> LobbyMap;

	FTimerHandle MatchTimerHandle;
	FTimerHandle ArrivalTimerHandle;
	bool bCountdownStarted = false;
};



