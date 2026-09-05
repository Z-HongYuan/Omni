// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "./DataTypes/ANA_EventDispatcher.h"

#include "Framework/Application/IInputProcessor.h"

class ANA_InputProcessor : public IInputProcessor
{
	ANA_InputProcessor() = default;

public:
	static void RegisterProcessor();
	static void UnregisterProcessor();

	static ANA_InputProcessor& Get() { return *instance; }

	ANA_EventDispatcher<ANA_TickHandler> tickDispatcher;
	ANA_EventDispatcher<ANA_KeyHandler> keyDownDispatcher;

	// static variables placed here for a common use of Event ID
	// be careful in usage by well copying the value in the capture like this:
	// [tickEventId_ = ANA_InputProcessor::tickEventId, keyDownEventId_ = ANA_InputProcessor::keyDownEventId] () { }
	static inline ANA_EventID tickEventId = 0;
	static inline ANA_EventID keyDownEventId = 0;

protected:
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

private:
	static TSharedPtr<ANA_InputProcessor> instance;
};
