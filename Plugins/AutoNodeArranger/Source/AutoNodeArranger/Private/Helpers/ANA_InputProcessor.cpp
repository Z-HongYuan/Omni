// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "ANA_InputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"

TSharedPtr<ANA_InputProcessor> ANA_InputProcessor::instance = nullptr;

void ANA_InputProcessor::RegisterProcessor()
{
	instance = MakeShareable(new ANA_InputProcessor());
	if (FSlateApplication::IsInitialized()) FSlateApplication::Get().RegisterInputPreProcessor(instance);
}

void ANA_InputProcessor::UnregisterProcessor()
{
	if (FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(instance);
	instance.Reset();
}

void ANA_InputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
	tickDispatcher.Execute(DeltaTime);
}

bool ANA_InputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	keyDownDispatcher.Execute(InKeyEvent);
	return false; // false to not consume event
}
