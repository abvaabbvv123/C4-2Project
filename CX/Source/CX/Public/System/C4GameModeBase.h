// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Base/C4Type.h"
#include "C4GameModeBase.generated.h"


UCLASS(Abstract)
class CX_API AC4GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AC4GameModeBase();

	void BroadcastChatMessage(APlayerController* Sender, const FString& Message);
	//void BroadcastSystemMessage(const FString& Message);
	void BroadcastInfo(const FC4Info& Info);

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual bool CanAcceptNewPlayer(FString& OutReason);

	UPROPERTY(EditDefaultsOnly, Category = "Server|Setting")
	int32 MaxPlayers = 2;

private:
	void SendChat(const FString& Sender, const FString& Message) const;
};