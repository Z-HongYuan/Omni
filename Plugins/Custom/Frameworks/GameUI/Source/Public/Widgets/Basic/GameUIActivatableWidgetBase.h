// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameUIActivatableWidgetBase.generated.h"

#define UE_API GAMEUI_API

/** 可激活界面使用的输入模式。 */
UENUM(BlueprintType)
enum class EGameUIWidgetInputMode : uint8
{
	// 沿用父类或蓝图提供的输入配置，默认不指定输入模式
	Default,

	// 界面与游戏都可以接收输入
	GameAndMenu,

	// 仅游戏接收输入
	Game,

	// 仅界面接收输入，不捕获鼠标
	Menu
};

/**
 * 最基础的可激活界面，统一管理输入模式、鼠标捕获和初始焦点校验
 */
UCLASS(MinimalAPI, Abstract)
class UGameUIActivatableWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// UCommonActivatableWidget 接口
	UE_API virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	// UCommonActivatableWidget 接口结束

#if WITH_EDITOR
	// UUserWidget 接口
	UE_API virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const override;
	// UUserWidget 接口结束
#endif

protected:
	/** 界面激活时使用的输入模式。 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	EGameUIWidgetInputMode InputConfig = EGameUIWidgetInputMode::Default;

	/** 允许游戏接收输入时使用的鼠标捕获方式。 */
	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(EditCondition="InputConfig == EGameUIWidgetInputMode::GameAndMenu || InputConfig == EGameUIWidgetInputMode::Game"))
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};

#undef UE_API
