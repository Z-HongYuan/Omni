// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TeamHelper.generated.h"

#define UE_API CUSTOMTEAM_API

class UTexture;
class UTeamDisplayAssetBase;

/** 为蓝图查询队伍归属和展示资产。 */
UCLASS(MinimalAPI)
class UTeamHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 输出对象的队伍 ID 和展示资产；未找到队伍时 ID 为 INDEX_NONE。 */
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(Keywords="GetTeamFromObject", DefaultToSelf="Agent", AdvancedDisplay="bLogIfNotSet"))
	static void FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UTeamDisplayAssetBase*& DisplayAsset, bool bLogIfNotSet = false);

	UFUNCTION(BlueprintCallable, Category=Teams, meta=(WorldContext="WorldContextObject"))
	static UTeamDisplayAssetBase* GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId);
};

#undef UE_API
