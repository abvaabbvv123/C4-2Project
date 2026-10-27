// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineBaseTypes.h"
#include "C4GameInstance.generated.h"
class UNetDriver;
/**
 * 
 */
enum class EC4ConnectIntent : uint8
{
	None,
	Hosting,
	Joining,
	Return
};

UCLASS(Config = Game)
class CX_API UC4GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	bool StartServer();
	bool JoinServer(const FString& Code, FString& OutError);
	bool ReturnAfterFail();
	void ConnectServer();
	void ToTitle();
	FString ChangeLastError();

	void SetNickname(const FString& NewName);
	const FString& GetNickname() const { return Nickname; }

	FString GetRoomCode() const { return EncodeRoomCode(CurrentCode); }
	static FString EncodeRoomCode(const FString& Address);
	static bool DecodeRoomCode(const FString& Code, FString& OutAddress);
	static FString FormatRoomCode(const FString& Code);

	static FString SetNewNickname(const FString& In);
	static constexpr int32 MaxNameLength = 12;

protected:
	UPROPERTY(Config) TArray<FString> ServerAddresses;
	UPROPERTY(Config) FString MainMenuMap = TEXT("/Game/Maps/MainMenu");

private:
	void ConnectTo(const FString& Address, bool bAsHost);
	bool TryNextHostAddress();

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	FString Nickname;
	FString LastError;
	FString CurrentCode;
	FString HomeCode;
	TArray<FString> HostAddress;
	EC4ConnectIntent Intent = EC4ConnectIntent::None;

	FDelegateHandle NetworkFailureHandle;
	FDelegateHandle TravelFailureHandle;
};