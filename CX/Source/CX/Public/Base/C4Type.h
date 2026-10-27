// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C4Type.generated.h"
/**
 * 
 */

UENUM(BlueprintType)
enum class EC4MatchState : uint8
{
	Waiting  UMETA(DisplayName = "Waiting"),
	Playing  UMETA(DisplayName = "Playing"),
	Finished UMETA(DisplayName = "Finished")
};
UENUM(BlueprintType)
enum class EC4InfoType : uint8
{
	PlayerJoined,
	PlayerLeft,
	NameChanged,
	AllReady,
	CountdownCanceled,
	MatchStartsIn,
	MatchStart,
	MatchOver,
	CrashCoinLost,
	MineCoinLost
};
USTRUCT(BlueprintType)
struct FC4Info
{
	GENERATED_BODY()

	UPROPERTY() EC4InfoType Type = EC4InfoType::MatchStart;
	UPROPERTY() int32 Value = 0;
	UPROPERTY() FString NameA;
	UPROPERTY() FString NameB;

	FC4Info() = default;
	FC4Info(EC4InfoType InType, int32 InValue = 0, const FString& InA = FString(), const FString& InB = FString())
		: Type(InType), Value(InValue), NameA(InA), NameB(InB) {
	}
};


namespace C4Text
{
	FText Info(const FC4Info& In);

	// Error in Lobby
	FText ServerFull();
	FText MatchInProgress();
	FText RoomOccupied();

	// Connect Server
	FText Version();
	FText FindingRoom();
	FText ConnectionFailed(const FString& Reason);
	FText NoServerConfigured();
	FText BadGameInstance();
	FText InvalidRoomCode();
	FText AlreadyInRoom();
	FText NoEmptyRoom();
	FText DefaultNickname();

	// Lobby
	FText RoomCode(const FString& FormattedCode);
	FText JoinFailed(const FString& Reason);
	FText JoiningRoom();
	FText NicknameEmpty();
	FText NicknameUpdated();
	FText CodeCopied();
	FText NoRoomCode();
	FText LobbyStartingIn(int32 Seconds);
	FText LobbyWaiting(int32 Count, int32 Required);
	FText LobbyReadyCount(int32 Ready, int32 Count);
	FText PlayerRow(const FString& Name, bool bLocal, bool bReady);
	FText ReadyButton(bool bLocalReady);

	// Match UI Info
	FText WaitingPlayers();
	FText StartingIn(int32 Seconds);
	FText MatchTime(int32 Seconds);
	FText ReturningIn(int32 Seconds);
	FText ResultWin();
	FText ResultDraw();
	FText ResultLose();
	FText WinnerEntry(const FString& Name, int32 Score); // "Name (Score)"
	FText Winners(const TArray<FString>& Lines);
	FText NoWinner();

	// Status Info
	FText Coins(int32 Amount);
	FText BonusSpeed(int32 Percent);
	FText Mines(int32 Amount);

	// Chatt
	FText ChatLine(const FString& Sender, const FString& Message);
}