// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Widgets/Basic/GameUIActivatableWidgetBase.h"

#if WITH_EDITOR
#include "Blueprint/WidgetTree.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIActivatableWidgetBase)

#define LOCTEXT_NAMESPACE "GameUI"

TOptional<FUIInputConfig> UGameUIActivatableWidgetBase::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EGameUIWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EGameUIWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EGameUIWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EGameUIWidgetInputMode::Default:
	default:
		return Super::GetDesiredInputConfig();
	}
}

#if WITH_EDITOR
void UGameUIActivatableWidgetBase::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// 不参与激活焦点的控件无需配置初始焦点。
	if (!SupportsActivationFocus()) return;

	// 蓝图可以动态返回焦点目标，也可以直接指定子控件作为默认焦点。
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(ThisClass, BP_GetDesiredFocusTarget))) return;
	if (const FName FocusWidgetName = GetDesiredFocusWidgetName(); !FocusWidgetName.IsNone() && BlueprintWidgetTree.FindWidget(FocusWidgetName)) return;

	if (GetParentNativeClass(GetClass()) == StaticClass())
	{
		CompileLog.Warning(LOCTEXT("MissingDesiredFocusTarget", "尚未配置初始焦点。请设置 Desired Focus Widget 或实现 GetDesiredFocusTarget，否则手柄导航可能无法正常工作。"));
	}
	else
	{
		// C++ 子类可能已经实现焦点逻辑，此时只提供提示。
		CompileLog.Note(LOCTEXT("CheckNativeDesiredFocusTarget", "尚未发现蓝图初始焦点配置。如果 C++ 父类已实现 NativeGetDesiredFocusTarget，可以忽略此提示。"));
	}
}
#endif

#undef LOCTEXT_NAMESPACE
