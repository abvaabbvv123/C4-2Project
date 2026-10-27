// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/CoreStyle.h"
#include "C4ChatWidget.generated.h"

class UEditableTextBox;
class UScrollBox;

UCLASS()
class CX_API UC4ChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void FocusChatInput(); // when open chat input with enter key

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type Method);
	UFUNCTION()
	void OnNewChatMessage(const FString& Sender, const FString& Message);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_ChatBox;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxChatLines = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	FSlateFontInfo ChatFont = FCoreStyle::GetDefaultFontStyle("Regular", 12);
};
