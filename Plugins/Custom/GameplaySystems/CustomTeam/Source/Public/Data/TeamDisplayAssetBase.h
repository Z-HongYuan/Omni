// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "TeamDisplayAssetBase.generated.h"

#define UE_API CUSTOMTEAM_API

/** 队伍展示资产基类，项目可在子类中添加名称、图标或配色等数据。 */
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


	// 具体展示字段由项目子类定义。
};

#undef UE_API
