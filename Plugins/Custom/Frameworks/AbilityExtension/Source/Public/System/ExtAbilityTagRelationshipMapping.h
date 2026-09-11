// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ExtAbilityTagRelationshipMapping.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 定义不同能力标签之间关系的结构 */
USTRUCT()
struct FExtAbilityTagRelationship
{
	GENERATED_BODY()

	/** 技能的身份 Tag */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** 技能激活时，其余哪些 Tag 的技能不能激活 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** 技能激活时，其余哪些 Tag 的技能被取消 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** 要激活技能，必须先有这些 Tag */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** 如果有这些 Tag，技能不能激活 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};


/**
 * 能力标签关系表,控制技能与其他技能之间的激活/取消关系
 */
UCLASS(MinimalAPI)
class UExtAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

	/** 不同游戏标签之间的关系列表（哪些标签会阻挡或取消其他标签）*/
	UPROPERTY(EditAnywhere, Category = Ability, meta=(TitleProperty="AbilityTag"))
	TArray<FExtAbilityTagRelationship> AbilityTagRelationships;

public:
	/*
	 * 给定一组能力标签，添加额外的标签以阻挡和取消
	 * 输入：当前激活的技能的 AbilityTag 集合
	 * 输出：应该 Block 哪些 Tag、Cancel 哪些 Tag
	 */
	UE_API void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/*
	 * 给定一组能力标签后，添加额外的必需和阻挡标签
	 * 输入：要激活的技能的 AbilityTag 集合
	 * 输出：额外需要满足的 Tag、额外禁止存在的 Tag
	 */
	UE_API void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	/*
	 * 如果指定的能力标签被传递的动作标签取消，则返回为真 
	 * 输入：当前正在运行的技能的 AbilityTag 集合 + 一个 ActionTag
	 * 输出：这个 ActionTag 代表的技能是否应该 Cancel 当前技能
	 */
	UE_API bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};
#undef UE_API
