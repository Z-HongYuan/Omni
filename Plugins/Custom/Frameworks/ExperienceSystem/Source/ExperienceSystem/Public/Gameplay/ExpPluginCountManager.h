// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "ExpPluginCountManager.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/**
 * Exp 插件计数管理器。
 * 协调同一编辑器进程中多个体验对 GF 插件的请求，最后一个请求者退出时才允许停用。
 * 仅在编辑器进程中计数，BeginPIE 由 Editor 模块转发以重置记录。
 * 非编辑器运行时不计数；这里只协调插件停用，不处理 Action 实例隔离或加载中退出。
 */
UCLASS(MinimalAPI)
class UExpPluginCountManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UE_API void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString& PluginURL);
	static bool RequestToDeactivatePlugin(const FString& PluginURL);
#else
	static void NotifyOfPluginActivation(const FString& PluginURL)
	{
	}
	static bool RequestToDeactivatePlugin(const FString& PluginURL) { return true; }
#endif

private:
	/** 按插件 URL 记录 PIE 世界的请求次数，归零后才允许停用。 */
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};

#undef UE_API
