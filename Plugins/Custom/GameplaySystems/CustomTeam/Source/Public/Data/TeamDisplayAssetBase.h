// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "TeamDisplayAssetBase.generated.h"

#define UE_API CUSTOMTEAM_API

/**
 * 在子类内拓展团队资产包含的数据
 */
UCLASS(MinimalAPI)
class UTeamDisplayAssetBase : public UDataAsset
{
	GENERATED_BODY()

public:
	//~UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~End of UObject interface


	/*
	 * 在子类内拓展团队资产包含的数据
	 * 1. 例如团队的名称
	 * 2. 例如团队的图标
	 * 3. 团队使用的公用资源,颜色,字体,材质等
	 */
};

#undef UE_API
