// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ExtAbilityTagRelationshipMapping.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 能力标签对应的激活、阻塞和取消条件。 */
USTRUCT()
struct FExtAbilityTagRelationship
{
	GENERATED_BODY()

	/** 用于匹配能力资产标签的标识。 */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** 此能力激活时需要阻塞的其他能力标签。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** 此能力激活时需要取消的其他能力标签。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** 激活能力前，ASC 必须拥有的标签。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** ASC 拥有任一标签时阻止能力激活。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};


/** 数据驱动的能力标签关系表，用于扩展激活条件及能力间的阻塞、取消关系。 */
UCLASS(MinimalAPI)
class UExtAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

	/** 各能力标签对应的关系规则。 */
	UPROPERTY(EditAnywhere, Category = Ability, meta=(TitleProperty="AbilityTag"))
	TArray<FExtAbilityTagRelationship> AbilityTagRelationships;

public:
	/** 按能力标签匹配规则，将阻塞和取消标签追加到非空输出容器。 */
	UE_API void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/** 按能力标签匹配规则，将额外的必需和阻塞条件追加到非空输出容器。 */
	UE_API void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	/** 检查与 ActionTag 精确对应的规则是否会取消具有 AbilityTags 的能力。 */
	UE_API bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};
#undef UE_API
