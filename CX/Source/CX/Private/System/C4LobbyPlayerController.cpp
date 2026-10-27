#include "System/C4LobbyPlayerController.h"
#include "System/C4LobbyGameMode.h"
#include "System/C4PlayerState.h"
#include "Engine/World.h"
#include "System/C4GameInstance.h"
#include "GameFramework/PlayerState.h"
void AC4LobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;

	if (UC4GameInstance* GI = GetGameInstance<UC4GameInstance>())
	{
		GI->ConnectServer();
	}
}
void AC4LobbyPlayerController::ToggleReady()
{
	if (!IsLocalController()) return;
	if (const AC4PlayerState* PS = GetPlayerState<AC4PlayerState>())
	{
		ServerSetReady(!PS->IsReady());
	}
}
void AC4LobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastReadyTime < ReadyCooldown) return;
	LastReadyTime = Now;

	AC4PlayerState* PS = GetPlayerState<AC4PlayerState>();
	if (!PS) return;
	PS->SetReady(bNewReady);

	if (AC4LobbyGameMode* GM = GetWorld()->GetAuthGameMode<AC4LobbyGameMode>())
	{
		GM->EvaluateCountdown();
	}
}
void AC4LobbyPlayerController::ChangeInputMode(bool bCursorMode)
{
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
	SetShowMouseCursor(true);
}
void AC4LobbyPlayerController::RequestNickname(const FString& NewName)
{
	if (!IsLocalController()) return;
	if (UC4GameInstance* GI = GetGameInstance<UC4GameInstance>())
	{
		GI->SetNickname(NewName);
		ServerSetNickname(GI->GetNickname());
	}
}
void AC4LobbyPlayerController::ServerSetNickname_Implementation(const FString& NewName)
{
	const FString Clean = UC4GameInstance::SetNewNickname(NewName);
	if (Clean.IsEmpty() || !PlayerState || Clean == PlayerState->GetPlayerName()) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastNicknameTime < NicknameCooldown) return;
	LastNicknameTime = Now;

	if (AC4LobbyGameMode* GM = GetWorld()->GetAuthGameMode<AC4LobbyGameMode>())
	{
		const FString OldName = PlayerState->GetPlayerName();
		GM->ChangeName(this, Clean, true);
		GM->BroadcastInfo(FC4Info(EC4InfoType::NameChanged, 0, OldName, Clean));
	}
}