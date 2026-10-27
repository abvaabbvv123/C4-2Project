#include "System/C4LobbyGameMode.h"
#include "System/C4LobbyPlayerController.h"
#include "System/C4GameState.h"
#include "System/C4PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AC4LobbyGameMode::AC4LobbyGameMode()
{
	PlayerControllerClass = AC4LobbyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
void AC4LobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	EvaluateCountdown();
}
void AC4LobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty()) return;

	if (UGameplayStatics::HasOption(Options, TEXT("Host")) && GetNumPlayers() > 0)
	{
		ErrorMessage = C4Text::RoomOccupied().ToString();
	}
}
void AC4LobbyGameMode::Logout(AController* Exiting)
{
	const APlayerState* LeaverPS = IsValid(Exiting) ? Exiting->PlayerState : nullptr;
	Super::Logout(Exiting);

	if (!GetWorld()->bIsTearingDown)
	{
		EvaluateCountdown(LeaverPS);
	}
}

void AC4LobbyGameMode::EvaluateCountdown(const APlayerState* Exclude)
{
	if (bTravelling) return;

	int32 PlayerCount = 0;
	const bool bCanStart = AreAllReady(Exclude, PlayerCount) && PlayerCount >= RequiredPlayers;

	if (bCanStart && !bCountingDown)       StartCountdown();
	else if (!bCanStart && bCountingDown)  CancelCountdown();
}

bool AC4LobbyGameMode::AreAllReady(const APlayerState* Exclude, int32& OutPlayerCount) const
{
	OutPlayerCount = 0;
	bool bAllReady = true;
	if (!GameState) return false;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		const AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS);
		if (!IsValid(C4PS) || PS == Exclude || PS->IsOnlyASpectator()) continue;

		++OutPlayerCount;
		if (!C4PS->IsReady()) bAllReady = false;
	}
	return bAllReady && OutPlayerCount > 0;
}
void AC4LobbyGameMode::StartCountdown()
{
	bCountingDown = true;
	if (AC4GameState* GS = GetGameState<AC4GameState>())
	{
		GS->SetRemainingTime(CountdownDuration);
	}
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ThisClass::TickCountdown, 1.f, true);
	BroadcastInfo(FC4Info(EC4InfoType::AllReady, CountdownDuration));
}

void AC4LobbyGameMode::CancelCountdown()
{
	bCountingDown = false;
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	if (AC4GameState* GS = GetGameState<AC4GameState>())
	{
		GS->SetRemainingTime(0);
	}
	BroadcastInfo(FC4Info(EC4InfoType::CountdownCanceled));
}

void AC4LobbyGameMode::TickCountdown()
{
	AC4GameState* GS = GetGameState<AC4GameState>();
	if (!GS) return;

	const int32 Next = GS->GetRemainingTime() - 1;
	GS->SetRemainingTime(FMath::Max(Next, 0));
	if (Next <= 0)
	{
		TravelToGame();
	}
}
void AC4LobbyGameMode::TravelToGame()
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	if (InGameMap.IsNull())
	{
		CancelCountdown();
		return;
	}
	int32 PlayerCount = 0;
	AreAllReady(nullptr, PlayerCount);

	bTravelling = true;
	GetWorld()->ServerTravel(FString::Printf(TEXT("%s?Players=%d"), *InGameMap.GetLongPackageName(), PlayerCount));
}