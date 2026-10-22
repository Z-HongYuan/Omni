// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "LoadingScreenDeveloperSettings.generated.h"

#define UE_API LOADINGSCREEN_API

/** 配置加载画面的控件、显示时长和调试开关。 */
UCLASS(Config = "Game", DefaultConfig, meta=(DisplayName="Loading Screen Setting"))
class ULoadingScreenDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	/** 返回 Project Settings 中的设置分类。 */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	/** 用于显示加载画面的控件类。 */
	UPROPERTY(config, EditAnywhere, Category="Display", meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath LoadingScreenWidget;

	/** 加载控件在视口中的显示层级。 */
	UPROPERTY(config, EditAnywhere, Category="Display")
	int32 LoadingScreenZOrder = 10000;

	/**
	 * 其他加载完成后额外保持画面的秒数，为纹理流送预留时间。
	 * 编辑器默认不应用此延迟，可通过 HoldLoadingScreenAdditionalSecsEvenInEditor 开启。
	 */
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s, ConsoleVariable="LoadingScreen.HoldLoadingScreenAdditionalSecs"))
	float HoldLoadingScreenAdditionalSecs = 2.0f;

	/** 加载检查点的挂起检测超时秒数；零表示不启用此超时。 */
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s))
	float LoadingScreenHeartbeatHangDuration = 0.0f;

	/** 记录加载画面仍未关闭原因的日志间隔秒数；零表示关闭周期日志。 */
	UPROPERTY(config, EditAnywhere, Category="Configuration", meta=(ForceUnits=s))
	float LogLoadingScreenHeartbeatInterval = 5.0f;

	/** 是否在编辑器中强制 Tick Slate，使加载画面立即刷新。 */
	UPROPERTY(config, EditAnywhere, Category="Configuration")
	bool ForceTickLoadingScreenEvenInEditor = true;

	/** 是否每帧记录加载画面显示或隐藏的原因。 */
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.LogLoadingScreenReasonEveryFrame"))
	bool LogLoadingScreenReasonEveryFrame = false;

	/** 强制显示加载画面，用于调试。 */
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.AlwaysShow"))
	bool ForceLoadingScreenVisible = false;

	/** 是否在编辑器中也应用 HoldLoadingScreenAdditionalSecs 延迟。 */
	UPROPERTY(Transient, EditAnywhere, Category="Debugging")
	bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

	/**
	 * 加载画面显示时，是否在编辑器中也拦截 Slate 输入。
	 * 默认值与 LoadingScreen.AlwaysStopPlayerInput 一致。
	 */
	UPROPERTY(Transient, EditAnywhere, Category="Debugging", meta=(ConsoleVariable="LoadingScreen.AlwaysStopPlayerInput"))
	bool ForceDisablePlayerInputInEditor = false;
};

#undef UE_API
