// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/AssetManager.h"
#include "OmniAssetManager.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目资产管理器，提供统一访问与初始加载入口，资源扫描沿用引擎流程。
 * 通过 DefaultEngine.ini 的 AssetManagerClassName 配置。
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
