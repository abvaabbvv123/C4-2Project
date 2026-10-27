// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/C4MainMenuWidget.h"
#include "System/C4GameInstance.h"
#include "Base/C4Type.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"


void UC4MainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Button_Connect->OnClicked.AddUniqueDynamic(this, &ThisClass::OnConnectClicked);
	Button_Quit->OnClicked.AddUniqueDynamic(this, &ThisClass::OnQuitClicked);

	ShowInfo(C4Text::Version());
	UC4GameInstance* GI = GetGameInstance<UC4GameInstance>();
	if (!GI) return;
	if (GI->ReturnAfterFail())
	{
		bConnecting = true;
		Button_Connect->SetIsEnabled(false);
		ShowInfo(C4Text::FindingRoom());
		return;
	}

	const FString Error = GI->ChangeLastError();
	if (!Error.IsEmpty())
	{
		ShowInfo(C4Text::ConnectionFailed(Error));
	}
}
void UC4MainMenuWidget::NativeDestruct()
{
	Button_Connect->OnClicked.RemoveDynamic(this, &ThisClass::OnConnectClicked);
	Button_Quit->OnClicked.RemoveDynamic(this, &ThisClass::OnQuitClicked);
	Super::NativeDestruct();
}
void UC4MainMenuWidget::OnConnectClicked()
{
	if (bConnecting) return;

	UC4GameInstance* GI = GetGameInstance<UC4GameInstance>();
	if (!GI)
	{
		ShowInfo(C4Text::BadGameInstance()); // Project Settings의 GameInstance 클래스 확인 안내
		return;
	}
	if (!GI->StartServer())
	{
		ShowInfo(C4Text::NoServerConfigured());
		return;
	}

	bConnecting = true;
	Button_Connect->SetIsEnabled(false);
	ShowInfo(C4Text::FindingRoom());
}

void UC4MainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
void UC4MainMenuWidget::ShowInfo(const FText& Message)
{
	TextBlock_Info->SetText(Message);
}