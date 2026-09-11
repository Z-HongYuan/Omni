// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/AssetManager.h"
#include "OmniAssetManager.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目资产管理器。
 * 当前提供统一访问与初始加载入口，资源扫描沿用引擎流程。
 * 在 DefaultEngine.ini 中通过 AssetManagerClassName 指定此类。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - GetAsset/GetSubclass 同步加载封装、常驻引用管理与加载诊断。
 * - GameData 配置与缓存、默认 PawnData 兜底，以及 PIE 前的 GameData 预加载。
 * - 带权启动任务与进度汇总；原版进度回调也尚未连接加载屏。
 * - GameplayCue 常驻预加载接线；AbilityExtension 已提供 LoadAlwaysLoadedCues，可优先复用。
 */
UCLASS(MinimalAPI, Config = Game)
class UOmniAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/** 返回引擎持有的项目资产管理器；配置类型错误时立即报错。 */
	static UE_API UOmniAssetManager& Get();

	//~UAssetManager interface
	UE_API virtual void StartInitialLoading() override;
};

#undef UE_API
