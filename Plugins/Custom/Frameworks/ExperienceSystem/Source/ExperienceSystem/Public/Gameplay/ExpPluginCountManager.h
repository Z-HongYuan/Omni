// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "ExpPluginCountManager.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/**
 * Exp 插件计数管理器。
 * 协调同一编辑器进程中多个体验对 GF 插件的请求，最后一个请求者退出时才允许停用。
 * 对应 LyraExperienceManager：保留其 WITH_EDITOR / GIsEditor 计数流程；BeginPIE 由 Editor 模块转发。
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
	// 一个 URL 对应多个 PIE 世界的请求，只有计数归零才实际停用插件。
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};

#undef UE_API
