// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "ExtAttributeSet.generated.h"

#define UE_API ABILITYEXTENSION_API

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct FGameplayEffectSpec;
class UExtAbilitySystemComponent;

/** 
 * 用于广播属性事件的委托，其中一些参数在客户端上可能为空：
 * @param EffectInstigator	本次变更的原发起人
 * @param EffectCauser		导致变化的物理演员
 * @param EffectSpec		此更改的完整效果规范
 * @param EffectMagnitude	原始数值，这是夹紧之前的数值
 * @param OldValue			属性更改前的值
 * @param NewValue			更改后的值
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FExtAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

/**
 * 基础的属性集
 */
UCLASS(MinimalAPI)
class UExtAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UE_API UExtAttributeSet();

	// UE_API virtual UWorld* GetWorld() const override;

	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const;
};

#undef UE_API
