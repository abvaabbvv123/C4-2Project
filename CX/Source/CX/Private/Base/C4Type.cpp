// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/C4Type.h"

#define LOCTEXT_NAMESPACE "C4Text"

namespace C4Text
{
	FText Info(const FC4Info& In)
	{
		const FText A = FText::FromString(In.NameA);
		const FText B = FText::FromString(In.NameB);
		const FText N = FText::AsNumber(In.Value);

		switch (In.Type)
		{
		case EC4InfoType::PlayerJoined:      return FText::Format(LOCTEXT("PlayerJoined", "{0} joined the game."), A);
		case EC4InfoType::PlayerLeft:        return FText::Format(LOCTEXT("PlayerLeft", "{0} left the game."), A);
		case EC4InfoType::NameChanged:       return FText::Format(LOCTEXT("NameChanged", "{0} is now {1}."), A, B);
		case EC4InfoType::AllReady:          return FText::Format(LOCTEXT("AllReady", "All ready! Starting in {0} seconds."), N);
		case EC4InfoType::CountdownCanceled: return LOCTEXT("CountdownCanceled", "Countdown canceled.");
		case EC4InfoType::MatchStartsIn:     return FText::Format(LOCTEXT("MatchStartsIn", "Match starts in {0} seconds."), N);
		case EC4InfoType::MatchStart:        return LOCTEXT("MatchStart", "Match start!");
		case EC4InfoType::MatchOver:         return FText::Format(LOCTEXT("MatchOver", "Match over! Returning to lobby in {0} seconds."), N);
		case EC4InfoType::CrashCoinLost:     return FText::Format(LOCTEXT("CrashCoinLost", "Crash! Lost {0} coins."), N);
		case EC4InfoType::MineCoinLost:      return FText::Format(LOCTEXT("MineCoinLost", "Mine! Lost {0} coins."), N);
		}
		return FText::GetEmpty();
	}

	// error in lobby
	FText ServerFull() { return LOCTEXT("ServerFull", "Server is full."); }
	FText MatchInProgress() { return LOCTEXT("MatchInProgress", "Match in progress."); }
	FText RoomOccupied() { return LOCTEXT("RoomOccupied", "Room occupied."); }

	// Connect Server
	FText Version() { return LOCTEXT("Version", "0.0.1"); }
	FText FindingRoom() { return LOCTEXT("FindingRoom", "Finding a room..."); }
	FText ConnectionFailed(const FString& Reason) { return FText::Format(LOCTEXT("ConnectionFailed", "Connection failed: {0}"), FText::FromString(Reason)); }
	FText NoServerConfigured() { return LOCTEXT("NoServerConfigured", "No server configured."); }
	FText BadGameInstance() { return LOCTEXT("BadGameInstance", "GameInstance class is not C4GameInstance."); }
	FText InvalidRoomCode() { return LOCTEXT("InvalidRoomCode", "Invalid room code."); }
	FText AlreadyInRoom() { return LOCTEXT("AlreadyInRoom", "You are already in this room."); }
	FText NoEmptyRoom() { return LOCTEXT("NoEmptyRoom", "No empty room available."); }
	FText DefaultNickname() { return LOCTEXT("DefaultNickname", "Anonymous"); }

	// lobvby
	FText RoomCode(const FString& FormattedCode)
	{
		return FText::Format(LOCTEXT("RoomCode", "Room Code: {0}"), FText::FromString(FormattedCode.IsEmpty() ? TEXT("-") : FormattedCode));
	}
	FText JoinFailed(const FString& Reason) { return FText::Format(LOCTEXT("JoinFailed", "Join failed: {0}"), FText::FromString(Reason)); }
	FText JoiningRoom() { return LOCTEXT("JoiningRoom", "Joining room..."); }
	FText NicknameEmpty() { return LOCTEXT("NicknameEmpty", "Please enter a nickname."); }
	FText NicknameUpdated() { return LOCTEXT("NicknameUpdated", "Nickname updated."); }
	FText CodeCopied() { return LOCTEXT("CodeCopied", "Room code copied."); }
	FText NoRoomCode() { return LOCTEXT("NoRoomCode", "No room code."); }
	FText LobbyStartingIn(int32 Seconds) { return FText::Format(LOCTEXT("LobbyStartingIn", "Starting in {0}"), FText::AsNumber(Seconds)); }
	FText LobbyWaiting(int32 Count, int32 Req) { return FText::Format(LOCTEXT("LobbyWaiting", "Waiting for players ({0}/{1})"), FText::AsNumber(Count), FText::AsNumber(Req)); }
	FText LobbyReadyCount(int32 Ready, int32 C) { return FText::Format(LOCTEXT("LobbyReadyCount", "Ready {0}/{1}"), FText::AsNumber(Ready), FText::AsNumber(C)); }
	FText PlayerRow(const FString& Name, bool bLocal, bool bReady)
	{
		return FText::Format(LOCTEXT("PlayerRow", "{0}{1}  -  {2}"),
			FText::FromString(Name),
			bLocal ? LOCTEXT("YouTag", " (You)") : FText::GetEmpty(),
			bReady ? LOCTEXT("Ready", "READY") : LOCTEXT("NotReady", "NOT READY"));
	}
	FText ReadyButton(bool bLocalReady) { return bLocalReady ? LOCTEXT("ReadyCancel", "Cancel") : LOCTEXT("ReadyOn", "Ready"); }

	// Matcch HUD
	FText WaitingPlayers() { return LOCTEXT("WaitingPlayers", "Waiting for players..."); }
	FText StartingIn(int32 Seconds) { return FText::Format(LOCTEXT("StartingIn", "Starting in {0}"), FText::AsNumber(Seconds)); }
	FText MatchTime(int32 Seconds) { return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60)); }
	FText ReturningIn(int32 Seconds) { return FText::Format(LOCTEXT("ReturningIn", "Returning to lobby in {0}"), FText::AsNumber(Seconds)); }
	FText ResultWin() { return LOCTEXT("ResultWin", "VICTORY"); }
	FText ResultDraw() { return LOCTEXT("ResultDraw", "DRAW"); }
	FText ResultLose() { return LOCTEXT("ResultLose", "DEFEAT"); }
	FText WinnerEntry(const FString& Name, int32 Score) { return FText::Format(LOCTEXT("WinnerEntry", "{0} ({1})"), FText::FromString(Name), FText::AsNumber(Score)); }
	FText Winners(const TArray<FString>& Lines) { return FText::Format(LOCTEXT("Winners", "Winner: {0}"), FText::FromString(FString::Join(Lines, TEXT(", ")))); }
	FText NoWinner() { return LOCTEXT("NoWinner", "No winner"); }

	// Status Info
	FText Coins(int32 Amount) { return FText::Format(LOCTEXT("Coins", "Coins: {0}"), FText::AsNumber(Amount)); }
	FText BonusSpeed(int32 Percent)
	{
		return FText::Format(LOCTEXT("BonusSpeed", "Bonus Speed: {0}{1}%"),
			FText::FromString(Percent >= 0 ? TEXT("+") : TEXT("")), FText::AsNumber(Percent));
	}
	FText Mines(int32 Amount) { return FText::Format(LOCTEXT("Mines", "Mines: {0}"), FText::AsNumber(Amount)); }

	// Chat
	FText ChatLine(const FString& Sender, const FString& Message)
	{
		return FText::Format(LOCTEXT("ChatLine", "{0}: {1}"), FText::FromString(Sender), FText::FromString(Message));
	}
}

#undef LOCTEXT_NAMESPACE