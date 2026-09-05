// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/Misc/GameSettingPressAnyKey.h"

#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingPressAnyKey)

class ICursor;

class FSettingsPressAnyKeyInputPreProcessor : public IInputProcessor
{
public:
	FSettingsPressAnyKeyInputPreProcessor()
	{
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		HandleKey(InKeyEvent.GetKey());
		return true;
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		return true;
	}

	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		HandleKey(MouseEvent.GetEffectingButton());
		return true;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return true;
	}

	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		HandleKey(MouseEvent.GetEffectingButton());
		return true;
	}

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override
	{
		if (InWheelEvent.GetWheelDelta() != 0)
		{
			const FKey Key = InWheelEvent.GetWheelDelta() < 0 ? EKeys::MouseScrollDown : EKeys::MouseScrollUp;
			HandleKey(Key);
		}
		return true;
	}

	DECLARE_MULTICAST_DELEGATE(FSettingsPressAnyKeyInputPreProcessorCanceled);
	FSettingsPressAnyKeyInputPreProcessorCanceled OnKeySelectionCanceled;

	DECLARE_MULTICAST_DELEGATE_OneParam(FSettingsPressAnyKeyInputPreProcessorKeySelected, FKey);
	FSettingsPressAnyKeyInputPreProcessorKeySelected OnKeySelected;

private:
	void HandleKey(const FKey& Key)
	{
		// 如果是 Esc、触摸或手柄按键，则取消此流程。
		if (Key == EKeys::LeftCommand || Key == EKeys::RightCommand)
		{
			// 忽略
		}
		else if (Key == EKeys::Escape || Key.IsTouch() || Key.IsGamepadKey())
		{
			OnKeySelectionCanceled.Broadcast();
		}
		else
		{
			OnKeySelected.Broadcast(Key);
		}
	}
};

UGameSettingPressAnyKey::UGameSettingPressAnyKey(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

void UGameSettingPressAnyKey::NativeOnActivated()
{
	Super::NativeOnActivated();

	bKeySelected = false;

	InputProcessor = MakeShared<FSettingsPressAnyKeyInputPreProcessor>();
	InputProcessor->OnKeySelected.AddUObject(this, &ThisClass::HandleKeySelected);
	InputProcessor->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled);
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);
}

void UGameSettingPressAnyKey::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
}

void UGameSettingPressAnyKey::HandleKeySelected(FKey InKey)
{
	if (!bKeySelected)
	{
		bKeySelected = true;
		Dismiss([this, InKey]()
		{
			OnKeySelected.Broadcast(InKey);
		});
	}
}

void UGameSettingPressAnyKey::HandleKeySelectionCanceled()
{
	if (!bKeySelected)
	{
		bKeySelected = true;
		Dismiss([this]()
		{
			OnKeySelectionCanceled.Broadcast();
		});
	}
}

void UGameSettingPressAnyKey::Dismiss(TFunction<void()> PostDismissCallback)
{
	// 延迟一个 tick，以确保输入处理完成。
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this, PostDismissCallback](float DeltaTime)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UGameSettingPressAnyKey_Dismiss);

		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);

		DeactivateWidget();

		PostDismissCallback();

		return false;
	}));
}
