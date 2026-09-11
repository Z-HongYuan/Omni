// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicalMaterialWithTags.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

/**
 * 带有Tag标签的物理材质
 */
UCLASS(MinimalAPI)
class UPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UE_API UPhysicalMaterialWithTags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 这些物理材料的游戏标签容器
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};
#undef UE_API
