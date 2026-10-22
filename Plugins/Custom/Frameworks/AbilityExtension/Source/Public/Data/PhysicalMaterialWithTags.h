// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicalMaterialWithTags.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 附带 GameplayTag 的物理材质，供命中效果收集目标标签。 */
UCLASS(MinimalAPI)
class UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UE_API UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 此物理材质提供的游戏标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};
#undef UE_API
