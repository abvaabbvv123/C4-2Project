// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4MainUI.h"
#include "CXUI.h"
#include "UI/C4ChatWidget.h"
#include "System/C4PlayerController.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

UCXUI* UC4MainUI::GetVehicleUI() const { return VehicleUI; }
UC4ChatWidget* UC4MainUI::GetChatWidget() const { return ChatWidget; }

void UC4MainUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (TextBlock_Info) TextBlock_Info->SetVisibility(ESlateVisibility::Collapsed);

	if (AC4PlayerController* PC = GetOwningPlayer<AC4PlayerController>())
	{
		PC->OnInfo.AddUObject(this, &ThisClass::HandleInfo);
	}
}

void UC4MainUI::NativeDestruct()
{
	if (AC4PlayerController* PC = GetOwningPlayer<AC4PlayerController>())
	{
		PC->OnInfo.RemoveAll(this);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InfoTimerHandle);
	}
	Super::NativeDestruct();
}

void UC4MainUI::HandleInfo(const FText& Text)
{
	if (!TextBlock_Info) return;
	TextBlock_Info->SetText(Text);
	TextBlock_Info->SetVisibility(ESlateVisibility::HitTestInvisible);
	OnInfoShown(Text);

	if (InfoDuration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(InfoTimerHandle, this, &ThisClass::HideInfo, InfoDuration, false);
	}
}

void UC4MainUI::HideInfo()
{
	if (TextBlock_Info) TextBlock_Info->SetVisibility(ESlateVisibility::Collapsed);
}