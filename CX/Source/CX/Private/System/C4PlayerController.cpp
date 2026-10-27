// Fill out your copyright notice in the Description page of Project Settings.


#include "System/C4PlayerController.h"
#include "System/C4GameModeBase.h"
#include "UI/C4ChatWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

namespace
{
	UC4ChatWidget* FindChatWidget(UUserWidget* Root)
	{
		UC4ChatWidget* Found = nullptr;
		if (Root && Root->WidgetTree)
		{
			Root->WidgetTree->ForEachWidget([&Found](UWidget* Widget)
				{
					if (!Found) Found = Cast<UC4ChatWidget>(Widget);
				});
		}
		return Found;
	}
}

void AC4PlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;
	CreateUI();
	ChangeInputMode(false);
}
void AC4PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext) Subsystem->AddMappingContext(InputMappingContext, 0);
	}
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (OpenChatAction) EIC->BindAction(OpenChatAction, ETriggerEvent::Started, this, &ThisClass::OpenChatInput);
		if (ESCAction) EIC->BindAction(ESCAction, ETriggerEvent::Started, this, &ThisClass::HandleESC);
	}
}
void AC4PlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	RemoveUI();
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}
void AC4PlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
	Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);
}
void AC4PlayerController::CreateUI()
{
	if (!IsLocalController() || !MainUIClass) return;
	if (IsValid(MainUI) && MainUI->IsInViewport()) return;
	MainUI = CreateWidget<UUserWidget>(this, MainUIClass);
	if (!IsValid(MainUI)) return;

	MainUI->AddToViewport();
	ChatWidget = FindChatWidget(MainUI);
	OnUICreated(MainUI);
}
void AC4PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveUI();
	Super::EndPlay(EndPlayReason);
}
void AC4PlayerController::RemoveUI()
{
	if (IsValid(MainUI))
	{
		MainUI->RemoveFromParent();
	}
	MainUI = nullptr;
	ChatWidget = nullptr;
	bChatInputOpen = false;
}
void AC4PlayerController::ChangeInputMode(bool bCursorMode)
{
	if (bCursorMode) 
	{
		SetInputMode(FInputModeUIOnly());
		SetShowMouseCursor(true);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
}

void AC4PlayerController::HandleESC()
{
	if (bChatInputOpen)
	{
		CloseChatInput();
	}
}

void AC4PlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	OnPSSet.Broadcast(PlayerState);
}

void AC4PlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnPSSet.Broadcast(PlayerState);
}


void AC4PlayerController::SubmitChatMessage(const FString& Message)
{
	if (!IsLocalController()) return;
	const FString Trimmed = Message.TrimStartAndEnd();
	if (!Trimmed.IsEmpty())
	{
		ToServerRPCChatMessage(Trimmed);
	}
}
void AC4PlayerController::ClientShowInfo_Implementation(const FC4Info& Info)
{
	OnInfo.Broadcast(C4Text::Info(Info));
}
void AC4PlayerController::ToServerRPCChatMessage_Implementation(const FString& Message)
{
	const FString Trimmed = Message.TrimStartAndEnd();
	if (Trimmed.IsEmpty() || Trimmed.Len() > MaxChatLength) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastChatTime < ChatCooldown) return;
	LastChatTime = Now;

	if (AC4GameModeBase* GM = GetWorld()->GetAuthGameMode<AC4GameModeBase>())
	{
		GM->BroadcastChatMessage(this, Trimmed);
	}
}
void AC4PlayerController::ToClientRPCChatMessage_Implementation(const FString& Sender, const FString& Message)
{
	OnChat.Broadcast(Sender, Message);
}
void AC4PlayerController::OpenChatInput()
{
	if (!IsLocalController() || !IsValid(ChatWidget) || bChatInputOpen) return;
	bChatInputOpen = true;

	SetInputMode(FInputModeGameAndUI());
	ChatWidget->FocusChatInput();
}
void AC4PlayerController::CloseChatInput()
{
	if (!bChatInputOpen) return;
	bChatInputOpen = false;
	ChangeInputMode(false);
}
	