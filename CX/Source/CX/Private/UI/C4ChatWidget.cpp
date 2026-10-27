// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/C4ChatWidget.h"
#include "System/C4PlayerController.h" 
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"


void UC4ChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EditableTextBox_ChatInput->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnChatInputTextCommitted);
	if (AC4PlayerController* CXPC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
		CXPC->OnChat.AddUniqueDynamic(this, &ThisClass::OnNewChatMessage);
	}
}
void UC4ChatWidget::NativeDestruct()
{
	EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);

	if (AC4PlayerController* CXPC = Cast<AC4PlayerController>(GetOwningPlayer()))
	{
		CXPC->OnChat.RemoveDynamic(this, &ThisClass::OnNewChatMessage);
	}
	Super::NativeDestruct();
}

void UC4ChatWidget::FocusChatInput()
{
	EditableTextBox_ChatInput->SetKeyboardFocus();
}
void UC4ChatWidget::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type Method)
{
	AC4PlayerController* CXPC = Cast<AC4PlayerController>(GetOwningPlayer());
	if (!IsValid(CXPC))	return;
	if (Method == ETextCommit::OnEnter)
	{
		const FString Message = Text.ToString();
		EditableTextBox_ChatInput->SetText(FText::GetEmpty());
		CXPC->SubmitChatMessage(Message);
		CXPC->CloseChatInput();
	}
	else if (Method == ETextCommit::OnCleared || Method == ETextCommit::OnUserMovedFocus)
	{
		EditableTextBox_ChatInput->SetText(FText::GetEmpty());
		CXPC->CloseChatInput();
	}
}
void UC4ChatWidget::OnNewChatMessage(const FString& Sender, const FString& Message)
{
	const bool bIsSystem = Sender.IsEmpty();
	if (Sender.IsEmpty()) return;

	UTextBlock* TextLine = nullptr;
	if (ScrollBox_ChatBox->GetChildrenCount() >= MaxChatLines)
	{
		TextLine = Cast<UTextBlock>(ScrollBox_ChatBox->GetChildAt(0));
		ScrollBox_ChatBox->RemoveChildAt(0);
	}
	if (!TextLine)
	{
		TextLine = WidgetTree->ConstructWidget<UTextBlock>();
		TextLine->SetAutoWrapText(true);
		if (ChatFont.HasValidFont())
		{
			TextLine->SetFont(ChatFont);
		}
	}
	const FText Line = C4Text::ChatLine(Sender, Message);
	TextLine->SetText(Line);

	ScrollBox_ChatBox->AddChild(TextLine);
	ScrollBox_ChatBox->ScrollToEnd();
}
