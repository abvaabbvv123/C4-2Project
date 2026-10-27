// Fill out your copyright notice in the Description page of Project Settings.

#include "System/C4GameInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "Base/C4Type.h"
namespace
{
	constexpr uint64 RoomCodeKey = 0x5A3C96E1B247ull;
	const TCHAR* NicknameSection = TEXT("C4.Player");
	const TCHAR* NicknameKey = TEXT("Nickname");
}


void UC4GameInstance::Init()
{
	Super::Init();
	if (IsDedicatedServerInstance() || !GEngine) return;

	NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);

	FString Saved;
	GConfig->GetString(NicknameSection, NicknameKey, Saved, GGameUserSettingsIni);
	Nickname = SetNewNickname(Saved.IsEmpty() ? FString(FPlatformProcess::UserName()) : Saved);
	if (Nickname.IsEmpty()) Nickname = C4Text::DefaultNickname().ToString();
}
void UC4GameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	}
	Super::Shutdown();
}
bool UC4GameInstance::StartServer()
{
	HostAddress = ServerAddresses;
	for (int32 i = HostAddress.Num() - 1; i > 0; --i)
	{
		HostAddress.Swap(i, FMath::RandRange(0, i));
	}
	Intent = EC4ConnectIntent::Hosting;
	LastError.Reset();
	return TryNextHostAddress();
}

bool UC4GameInstance::TryNextHostAddress()
{
	if (HostAddress.IsEmpty()) return false;
	ConnectTo(HostAddress.Pop(), true);
	return true;
}
bool UC4GameInstance::JoinServer(const FString& Code, FString& OutError)
{
	FString Address;
	if (!DecodeRoomCode(Code, Address))
	{
		OutError = C4Text::InvalidRoomCode().ToString();
		return false;
	}
	if (EncodeRoomCode(Address) == GetRoomCode())
	{
		OutError = C4Text::AlreadyInRoom().ToString();
		return false;
	}

	Intent = EC4ConnectIntent::Joining;
	HostAddress.Reset();
	LastError.Reset();
	ConnectTo(Address, false);
	return true;
}
bool UC4GameInstance::ReturnAfterFail()
{
	switch (Intent)
	{
	case EC4ConnectIntent::Hosting:
		if (TryNextHostAddress()) return true;
		LastError = C4Text::NoEmptyRoom().ToString();
		break;

	case EC4ConnectIntent::Joining:
		if (!HomeCode.IsEmpty())
		{
			Intent = EC4ConnectIntent::Return;
			ConnectTo(HomeCode, false);
			return true;
		}
		break;

	default:
		break;
	}

	Intent = EC4ConnectIntent::None;
	return false;
}
void UC4GameInstance::ConnectServer()
{
	Intent = EC4ConnectIntent::None;
	HostAddress.Reset();
	HomeCode = CurrentCode;
}

void UC4GameInstance::ConnectTo(const FString& Address, bool bAsHost)
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC) return;

	CurrentCode = Address;
	FString URL = FString::Printf(TEXT("%s?Name=%s"), *Address, *Nickname);
	if (bAsHost)
	{
		URL += TEXT("?Host=1");
	}
	PC->ClientTravel(URL, TRAVEL_Absolute);
}
void UC4GameInstance::ToTitle()
{
	Intent = EC4ConnectIntent::None;
	HomeCode.Reset();
	CurrentCode.Reset();
	UGameplayStatics::OpenLevel(this, FName(*MainMenuMap));
}
FString UC4GameInstance::ChangeLastError()
{
	FString Out = MoveTemp(LastError);
	LastError.Reset();
	return Out;
}

void UC4GameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	LastError = ErrorString.IsEmpty() ? FString(ENetworkFailure::ToString(FailureType)) : ErrorString;
}

void UC4GameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	LastError = ErrorString.IsEmpty() ? FString(ETravelFailure::ToString(FailureType)) : ErrorString;
}

void UC4GameInstance::SetNickname(const FString& NewName)
{
	const FString NewNickName = SetNewNickname(NewName);
	if (NewNickName.IsEmpty()) return;

	Nickname = NewNickName;
	GConfig->SetString(NicknameSection, NicknameKey, *Nickname, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

FString UC4GameInstance::SetNewNickname(const FString& In)
{
	FString Out = In.TrimStartAndEnd();
	for (const TCHAR* check : { TEXT("?"), TEXT("="), TEXT("#"), TEXT("&") })
	{
		Out.ReplaceInline(check, TEXT(""));
	}
	Out.ReplaceInline(TEXT(" "), TEXT("_"));
	return Out.Left(MaxNameLength);
}

FString UC4GameInstance::EncodeRoomCode(const FString& Address)
{
	FString HostStr, PortStr;
	if (!Address.Split(TEXT(":"), &HostStr, &PortStr))
	{
		HostStr = Address;
		PortStr = TEXT("7777");
	}
	if (HostStr.Equals(TEXT("localhost"), ESearchCase::IgnoreCase))
	{
		HostStr = TEXT("127.0.0.1");
	}

	TArray<FString> Octets;
	HostStr.ParseIntoArray(Octets, TEXT("."));
	if (Octets.Num() != 4) return FString();

	uint64 Value = 0;
	for (const FString& Octet : Octets)
	{
		if (!Octet.IsNumeric()) return FString();
		const int32 N = FCString::Atoi(*Octet);
		if (N < 0 || N > 255) return FString();
		Value = (Value << 8) | static_cast<uint64>(N);
	}

	const int32 Port = FCString::Atoi(*PortStr);
	if (Port <= 0 || Port > 65535) return FString();

	Value = ((Value << 16) | static_cast<uint64>(Port)) ^ RoomCodeKey;
	return FString::Printf(TEXT("%015llu"), Value);
}

bool UC4GameInstance::DecodeRoomCode(const FString& Code, FString& OutAddress)
{
	FString Digits;
	for (const TCHAR C : Code)
	{
		if (FChar::IsDigit(C)) Digits.AppendChar(C);
	}
	if (Digits.Len() != 15) return false;

	const uint64 Value = FCString::Strtoui64(*Digits, nullptr, 10) ^ RoomCodeKey;
	if ((Value >> 48) != 0) return false;

	const uint32 Port = static_cast<uint32>(Value & 0xFFFF);
	const uint32 IP = static_cast<uint32>(Value >> 16);
	if (Port == 0) return false;

	OutAddress = FString::Printf(TEXT("%u.%u.%u.%u:%u"),
		(IP >> 24) & 0xFF, (IP >> 16) & 0xFF, (IP >> 8) & 0xFF, IP & 0xFF, Port);
	return true;
}

FString UC4GameInstance::FormatRoomCode(const FString& Code)
{
	FString Out;
	for (int32 i = 0; i < Code.Len(); ++i)
	{
		if (i > 0 && i % 3 == 0) Out.AppendChar(TEXT('-'));
		Out.AppendChar(Code[i]);
	}
	return Out;
}