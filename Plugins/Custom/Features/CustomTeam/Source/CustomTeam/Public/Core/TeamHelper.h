// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TeamHelper.generated.h"

#define UE_API CUSTOMTEAM_API

class UTexture;
class UTeamDisplayAssetBase;

/**
 * 队伍系统的帮助函数
 */
UCLASS(MinimalAPI)
class UTeamHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 返回该对象所属的队伍，若不属于队伍则返回INDEX_NONE
	UFUNCTION(BlueprintCallable, Category=Teams, meta=(Keywords="GetTeamFromObject", DefaultToSelf="Agent", AdvancedDisplay="bLogIfNotSet"))
	static void FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UTeamDisplayAssetBase*& DisplayAsset, bool bLogIfNotSet = false);

	UFUNCTION(BlueprintCallable, Category=Teams, meta=(WorldContext="WorldContextObject"))
	static UTeamDisplayAssetBase* GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId);
};

#undef UE_API
