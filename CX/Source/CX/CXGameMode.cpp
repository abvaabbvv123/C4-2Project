// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "System/C4GameState.h"
#include "System/C4PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
}
void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(ArrivalTimerHandle, this, &ThisClass::OnArrivalTimeout, ArrivalTimeout, false);
}
void ACXGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	ArrivedPlayers.AddUnique(NewPlayer);
	TryStartCountdown();
}
void ACXGameMode::TryStartCountdown()
{
	if (bCountdownStarted) return;
	if (CountArrived() < ExpectedPlayers) return;
	BeginCountdown();
}

void ACXGameMode::OnArrivalTimeout()
{
	if (bCountdownStarted) return;
	if (CountPlayers(nullptr) > 0) BeginCountdown();
	else ReturnToLobby();
}
void ACXGameMode::BeginCountdown()
{
	bCountdownStarted = true;
	GetWorldTimerManager().ClearTimer(ArrivalTimerHandle);
	EnterPhase(EC4MatchState::Waiting, WaitingDuration);
	BroadcastInfo(FC4Info(EC4InfoType::MatchStartsIn, WaitingDuration));
}
void ACXGameMode::EnterPhase(EC4MatchState NewState, int32 Duration)
{
	if (AC4GameState* GS = GetGameState<AC4GameState>())
	{
		GS->SetRemainingTime(Duration);
		GS->SetMatchState(NewState);
	}
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ThisClass::TickMatchTimer, 1.f, true);
}
void ACXGameMode::TickMatchTimer()
{
	AC4GameState* GS = GetGameState<AC4GameState>();
	if (!GS) return;

	const int32 Next = GS->GetRemainingTime() - 1;
	GS->SetRemainingTime(FMath::Max(Next, 0));
	if (Next > 0) return;

	switch (GS->GetMatchState())
	{
	case EC4MatchState::Waiting: StartMatch(); break;
	case EC4MatchState::Playing: EndMatch(); break;
	case EC4MatchState::Finished: ReturnToLobby(); break;
	}
}
void ACXGameMode::ReturnToLobby()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	if (LobbyMap.IsNull()) return;
	GetWorld()->ServerTravel(LobbyMap.GetLongPackageName());
}
void ACXGameMode::Logout(AController* Exiting)
{
	const APlayerState* LeaverPS = IsValid(Exiting) ? Exiting->PlayerState : nullptr;
	ArrivedPlayers.Remove(Cast<APlayerController>(Exiting));
	Super::Logout(Exiting); 

	if (!bCountdownStarted || GetWorld()->bIsTearingDown) return;
	if (CountPlayers(LeaverPS) <= 1)
	{
		EndMatch(LeaverPS);
	}
}
void ACXGameMode::StartMatch()
{
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS))
		{
			C4PS->ResetMatchStats();
		}
	}
	EnterPhase(EC4MatchState::Playing, MatchDuration);
	BroadcastInfo(FC4Info(EC4InfoType::MatchStart));
}
void ACXGameMode::EndMatch(const APlayerState* Leaver)
{
	AC4GameState* GS = GetGameState<AC4GameState>();
	if (!GS || GS->GetMatchState() == EC4MatchState::Finished) return;

	int32 BestScore = MIN_int32;
	TArray<TObjectPtr<APlayerState>> Winners;
	for (APlayerState* PS : GS->PlayerArray)
	{
		const AC4PlayerState* C4PS = Cast<AC4PlayerState>(PS);
		if (!C4PS || PS == Leaver) continue;

		const int32 Score = C4PS->GetTotalScore();
		if (Score > BestScore)
		{
			BestScore = Score;
			Winners.Reset();
		}
		if (Score == BestScore) Winners.Add(PS);
	}

	GS->SetWinners(Winners);
	EnterPhase(EC4MatchState::Finished, ResultDuration);
	BroadcastInfo(FC4Info(EC4InfoType::MatchOver, ResultDuration));
}
bool ACXGameMode::CanAcceptNewPlayer(FString& OutReason)
{
	if (!Super::CanAcceptNewPlayer(OutReason)) return false;
	if (bCountdownStarted)
	{
		OutReason = C4Text::MatchInProgress().ToString();
		return false;
	}
	return true;
}
int32 ACXGameMode::CountPlayers(const APlayerState* Exclude) const
{
	int32 Count = 0;
	if (GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (IsValid(PS) && PS != Exclude && !PS->IsOnlyASpectator()) ++Count;
		}
	}
	return Count;
}
AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalf);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}
void ACXGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	ExpectedPlayers = UGameplayStatics::GetIntOption(Options, TEXT("Players"), MinPlayersToStart);
	ExpectedPlayers = FMath::Clamp(ExpectedPlayers, 1, MaxPlayers);
}
int32 ACXGameMode::CountArrived()
{
	ArrivedPlayers.RemoveAll([](const TWeakObjectPtr<APlayerController>& PC) { return !PC.IsValid(); });
	return ArrivedPlayers.Num();
}