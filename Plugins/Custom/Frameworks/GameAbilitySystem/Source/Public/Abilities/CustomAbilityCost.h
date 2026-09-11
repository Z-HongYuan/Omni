// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "CustomGameplayAbility.h"
#include "CustomAbilityCost.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

struct FGameplayTagContainer;
struct FGameplayAbilityActorInfo;

/**
 * 自定义的消耗类,在 CustomAbility 中使用
 * 用于确定判断消耗的产生和使用
 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UCustomAbilityCost : public UObject
{
	GENERATED_BODY()

public:
	UCustomAbilityCost() { ; }

	// 检查我们是否负担得起这笔费用。
	// 可以将故障原因标签添加到OptionalRelevantTags（如果非空）中，可以查询该标签
	// 在其他地方确定如何提供用户反馈（例如，如果武器弹药耗尽，会发出咔嗒声）
	// Ability和ActorInfo在输入时保证不为null，但OptionalRelevantTag可以为nullptr。
	// @return 如果我们可以为该能力付费，则返回true，否则返回false。
	virtual bool CheckCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	/**
	 * 将能力成本应用于目标
	 *
	 * 笔记：
	 * - 你的实现不需要检查ShouldOnlyApplyCostOnHit（），调用者会为你做这件事。
	  * - 输入时，Ability和ActorInfo保证不为空。
	 */
	virtual void ApplyCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}

	/**如果为真，则只有当此能力成功命中时，才应应用此成本 */
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	/** 如果为真，则只有当此能力成功命中时，才应应用此成本 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	bool bOnlyApplyCostOnHit = false;
};

#undef UE_API
