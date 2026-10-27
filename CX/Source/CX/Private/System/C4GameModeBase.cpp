// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4GameModeBase.h"
#include "System/C4PlayerController.h"
#include "System/C4PlayerState.h"
#include "System/C4GameState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

AC4GameModeBase::AC4GameModeBase()
{
	PlayerStateClass = AC4PlayerState::StaticClass();
	GameStateClass = AC4GameState::StaticClass();
	bUseSeamlessTravel = true;
}

void AC4GameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty()) return;

	FString Reason;
	if (!CanAcceptNewPlayer(Reason))
	{
		ErrorMessage = Reason;
	}
}
bool AC4GameModeBase::CanAcceptNewPlayer(FString& OutReason)
{
	if (GetNumPlayers() >= MaxPlayers)
	{
		OutReason = C4Text::ServerFull().ToString();
		return false;
	}
	return true;
}
void AC4GameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	if (IsValid(NewPlayer) && IsValid(NewPlayer->PlayerState))
	{
		BroadcastInfo(FC4Info(EC4InfoType::PlayerJoined, 0, NewPlayer->PlayerState->GetPlayerName()));
	}
}
void AC4GameModeBase::Logout(AController* Exiting)
{
	const UWorld* World = GetWorld();
	if (IsValid(World) && !World->bIsTearingDown && IsValid(Exiting) && IsValid(Exiting->PlayerState))
	{
		BroadcastInfo(FC4Info(EC4InfoType::PlayerLeft, 0, Exiting->PlayerState->GetPlayerName()));
	}
	Super::Logout(Exiting);
}
void AC4GameModeBase::BroadcastInfo(const FC4Info& Info)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AC4PlayerController* PC = Cast<AC4PlayerController>(It->Get()))
		{
			PC->ClientShowInfo(Info);
		}
	}
}
void AC4GameModeBase::BroadcastChatMessage(APlayerController* Sender, const FString& Message)
{
	if (!IsValid(Sender) || !IsValid(Sender->PlayerState)) return;
	SendChat(Sender->PlayerState->GetPlayerName(), Message);
}
//void AC4GameModeBase::BroadcastSystemMessage(const FString& Message)
//{
	//SendChat(FString(), Message);
//}
void AC4GameModeBase::SendChat(const FString& Sender, const FString& Message) const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AC4PlayerController* PC = Cast<AC4PlayerController>(It->Get()))
		{
			PC->ToClientRPCChatMessage(Sender, Message);
		}
	}
}
