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
 * 广播属性变化及效果上下文；客户端的来源对象和 EffectSpec 可能为空。
 * EffectInstigator 为原始发起者，EffectCauser 为直接来源 Actor。
 * EffectMagnitude 为范围限制前的效果幅值，OldValue 和 NewValue 为变化前后的属性值。
 */
DECLARE_MULTICAST_DELEGATE_SixParams(FExtAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

/** 提供扩展 ASC 访问接口的属性集基类。 */
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
