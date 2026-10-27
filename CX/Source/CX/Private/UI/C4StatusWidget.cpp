// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4StatusWidget.h"
#include "System/C4PlayerController.h" 
#include "System/C4PlayerState.h"
#include "Components/TextBlock.h"
#include "Base/C4Type.h"

void UC4StatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AC4PlayerController* PC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
		PC->OnPSSet.AddUObject(this, &ThisClass::HandlePlayerStateSet);
		HandlePlayerStateSet(PC->PlayerState);
	}
}
void UC4StatusWidget::NativeDestruct()
{
	UnbindPlayerState();

	if (AC4PlayerController* PC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
		PC->OnPSSet.RemoveAll(this);
	}

	Super::NativeDestruct();
}
void UC4StatusWidget::HandlePlayerStateSet(APlayerState* NewPlayerState)
{
	BindPlayerState(Cast<AC4PlayerState>(NewPlayerState));
}
void UC4StatusWidget::BindPlayerState(AC4PlayerState* NewPlayerState)
{
	if (BoundPlayerState.Get() == NewPlayerState) return; 

	UnbindPlayerState();

	if (!IsValid(NewPlayerState)) return;

	BoundPlayerState = NewPlayerState;
	NewPlayerState->OnCoinsChanged.AddUObject(this, &ThisClass::HandleCoinsChanged);
	NewPlayerState->OnCurrentBonusSpeedChanged.AddUObject(this, &ThisClass::HandleCurrentBonusChanged);
	NewPlayerState->OnMineCountChanged.AddUObject(this, &ThisClass::HandleMineChanged);

	HandleCoinsChanged(NewPlayerState->GetCoins());
	HandleCurrentBonusChanged(NewPlayerState->GetCurrentBonusSpeed());
	HandleMineChanged(NewPlayerState->GetMineCount());
}
void UC4StatusWidget::UnbindPlayerState()
{
	if (AC4PlayerState* PS = BoundPlayerState.Get())
	{
		PS->OnCoinsChanged.RemoveAll(this);
		PS->OnCurrentBonusSpeedChanged.RemoveAll(this);
		PS->OnMineCountChanged.RemoveAll(this);
	}
	BoundPlayerState.Reset();
}
void UC4StatusWidget::HandleCurrentBonusChanged(int32 Amount)
{
	TextBlock_BonusSpeed->SetText(C4Text::BonusSpeed(Amount));
}
void UC4StatusWidget::HandleMineChanged(int32 Amount)
{
	TextBlock_MineCount->SetText(C4Text::Mines(Amount));
}
void UC4StatusWidget::HandleCoinsChanged(int32 NewCoins)
{
	TextBlock_Coins->SetText(C4Text::Coins(NewCoins));
}
