// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ExpPawnData.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UCameraAsset;
class UExtInputConfig;
class UExtAbilityTagRelationshipMapping;
class UExtAbilitySet;

/** Pawn 的静态配置，包含角色类型、能力、输入及相机资产。 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExpPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 生成 Pawn 时使用的类型。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass = nullptr;

	/** 由 PlayerState 授予 ASC 的能力集。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UExtAbilitySet>> AbilitySets;

	/** ASC 的能力标签关系映射，用于控制能力间的阻塞与取消关系。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UExtAbilityTagRelationshipMapping> TagRelationshipMapping = nullptr;

	/** 由输入组件消费的 Pawn 输入配置。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UExtInputConfig> InputConfig = nullptr;

	/** 由项目相机管理组件应用到 GameplayCamera 的相机资产。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraAsset> CameraAsset = nullptr;
};
#undef UE_API
