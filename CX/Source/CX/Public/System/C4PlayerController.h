// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Base/C4Type.h"
#include "C4PlayerController.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChat, const FString&, Sender, const FString&, Message);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPSSet, APlayerState*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInfo, const FText&);
class APlayerState;
class UUserWidget;
class UC4ChatWidget;
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS(Abstract)
class CX_API AC4PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	FOnPSSet OnPSSet;
	FOnInfo OnInfo;
	UPROPERTY(BlueprintAssignable)	FOnChat OnChat;

	virtual void InitPlayerState() override; 
	virtual void OnRep_PlayerState() override;
	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;
	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

	void SubmitChatMessage(const FString& Message);

	UFUNCTION(Server, Reliable) void ToServerRPCChatMessage(const FString& Message);
	UFUNCTION(Client, Reliable) void ToClientRPCChatMessage(const FString& Sender, const FString& Message);
	UFUNCTION(Client, Reliable)	void ClientShowInfo(const FC4Info& Info);

	void OpenChatInput();
	void CloseChatInput();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	void CreateUI();
	virtual void OnUICreated(UUserWidget* InMainUI) {}
	virtual void ChangeInputMode(bool bCursorMode);
	void HandleESC();
	void RemoveUI();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainUIClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> MainUI;
	UPROPERTY()
	TObjectPtr<UC4ChatWidget> ChatWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> OpenChatAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ESCAction;
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	float ChatCooldown = 0.5f;

	static constexpr int32 MaxChatLength = 64;
	float LastChatTime = -1000.f;
	bool bChatInputOpen = false;
};