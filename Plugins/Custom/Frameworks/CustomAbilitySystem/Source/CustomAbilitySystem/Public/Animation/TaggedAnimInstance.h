// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "TaggedAnimInstance.generated.h"

#define UE_API CUSTOMABILITYSYSTEM_API

/**
 * 带有 Tag 绑定的 AnimInstance
 */
UCLASS(MinimalAPI)
class UTaggedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UE_API UTaggedAnimInstance(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void NativeInitializeAnimation() override;

	UE_API virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	// 可以映射到蓝图变量的游戏标签。变量会随着标签的添加或移除自动更新。这些应该用来代替手动查询游戏标签。
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};

#undef UE_API
