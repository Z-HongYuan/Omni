// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatureAction.h"
#include "UObject/SoftObjectPath.h"

#include "GameFeatureAction_AddGameplayCuePath.generated.h"

#define UE_API OMNIGAME_API

/**
 * 把 GameplayCue 资源目录注册进 GameplayCueManager 的 GameFeatureAction
 *
 * 注意：
 * - 本 Action 只在数据里登记路径；真正的注册/注销发生在 OmniGameFeaturePolicy 内的
 *   UOmniGameFeature_AddGameplayCuePaths 观察者里（GameFeature Registering/Unregistering 时机）
 *
 * @see UAbilitySystemGlobals::GameplayCueNotifyPaths
 * 与 Lyra 的差异: 仅本地化命名与注释，逻辑与 Lyra 完全一致
 */
UCLASS(MinimalAPI, meta = (DisplayName = "添加 GameplayCue 路径"))
class UGameFeatureAction_AddGameplayCuePath final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	UE_API UGameFeatureAction_AddGameplayCuePath();

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	const TArray<FDirectoryPath>& GetDirectoryPathsToAdd() const { return DirectoryPathsToAdd; }

private:
	// 要注册给 GameplayCueManager 的路径列表（相对游戏 Content 目录）
	UPROPERTY(EditAnywhere, Category = "Game Feature | Gameplay Cues", meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> DirectoryPathsToAdd;
};

#undef UE_API
