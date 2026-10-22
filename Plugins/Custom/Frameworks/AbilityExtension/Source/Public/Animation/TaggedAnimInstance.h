// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "TaggedAnimInstance.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 将 ASC 的标签变化映射到动画蓝图属性。 */
UCLASS(MinimalAPI)
class UTaggedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UE_API UTaggedAnimInstance(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void NativeInitializeAnimation() override;

	/** 绑定有效 ASC；动画初始化时未找到 ASC 的调用方应在其就绪后补调。 */
	UE_API virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	/** 将游戏标签映射到蓝图变量，标签变化时自动更新变量。 */
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};

#undef UE_API
