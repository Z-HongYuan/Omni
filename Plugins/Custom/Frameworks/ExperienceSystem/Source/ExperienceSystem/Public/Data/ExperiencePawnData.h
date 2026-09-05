// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Core/CameraAssetReference.h"
#include "Engine/DataAsset.h"
#include "ExperiencePawnData.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UCustomInputConfig;
class UCustomAbilityTagRelationshipMapping;
class UCustomAbilitySet;

/**
 * 存储Pawn使用的各种必要数据
 * 静态数据
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExperiencePawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 使用的Pawn类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass = nullptr;

	// 使用的能力集/AbilitySet
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TObjectPtr<UCustomAbilitySet>> AbilitySets;

	// Pawn 使用的能力标签关系映射(控制能力之间的关系)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UCustomAbilityTagRelationshipMapping> TagRelationshipMapping = nullptr;

	// Pawn 所使用的输入配置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UCustomInputConfig> InputConfig = nullptr;

	// 决定采用GameplayCamera的摄像机资产
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraAsset> CameraAsset = nullptr;
};
#undef UE_API
