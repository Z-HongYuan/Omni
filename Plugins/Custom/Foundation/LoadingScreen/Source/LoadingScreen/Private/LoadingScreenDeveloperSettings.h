// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "LoadingScreenDeveloperSettings.generated.h"

#define UE_API LOADINGSCREEN_API

/**
 * 加载屏幕的设置
 */
UCLASS(Config = "Game", DefaultConfig, meta=(DisplayName="Loading Screen Setting"))
class ULoadingScreenDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	// 用于加载屏幕的控件。
	UPROPERTY(config, EditAnywhere, Category="Display", meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	// 加载屏幕控件在视口堆栈中的 Z 轴顺序
	UPROPERTY(config, EditAnywhere, Category="Display")
	int32 LoadingScreenZOrder = 10000;

	// 在其他加载完成后保持加载屏幕显示的额外时长（秒），以尝试给纹理流送一个机会来避免模糊
	// 注意：为了迭代时间，这在编辑器中通常不应用，但可以通过
	// HoldLoadingScreenAdditionalSecsEvenInEditor 启用
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s, ConsoleVariable="LoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	// 超过此间隔（秒）后，加载屏幕将被视为永久挂起（如果非零）。
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	// 每次记录是什么导致加载屏幕保持显示的间隔时间（秒）（如果非零）。
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	// 即使在编辑器中也强制 Tick 加载屏幕
	//（在迭代加载屏幕时有用）
	UPROPERTY(config, EditAnywhere, Category="Configuration")
	bool ForceTickLoadingScreenEvenInEditor = true;

	// 当为 true 时，每帧都会将加载屏幕显示或隐藏的原因打印到日志。
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = false;

	// 强制显示加载屏幕（用于调试）
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	// 即使在编辑器中也应用额外的 HoldLoadingScreenAdditionalSecs 延迟
	//（在迭代加载屏幕时有用）
	UPROPERTY(Transient, EditAnywhere, Category="Debugging")
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	// 加载画面显示时，在编辑器中也拦截 Slate 输入，用于手动验证输入阻塞。
	// 默认值与 LoadingScreen.AlwaysStopPlayerInput 保持一致。
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.AlwaysStopPlayerInput"))
	bool ForceDisablePlayerInputInEditor = false;
};

#undef UE_API
