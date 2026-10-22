// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Attributes/ExtAttributeSet.h"
#include "ExtHealthSet.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 维护生命值、生命上限及伤害、治疗元属性，复制生命值和上限。
 * GE 结算时消费并清零元属性；生命归零时通过委托传递效果上下文。
 * 伤害免疫拦截正数 MetaDamage，自毁伤害不受此限制。
 */
UCLASS(MinimalAPI, BlueprintType)
class UExtHealthSet : public UExtAttributeSet
{
	GENERATED_BODY()

public:
	UE_API UExtHealthSet();
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UExtHealthSet, Health)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MetaDamage)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MetaHealing)

	/** 只在 GE 结算归零时发送；直接设置属性仍通过 ASC 属性变化委托通知。 */
	mutable FExtAttributeEvent OnOutOfHealth;

	UE_API virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	UE_API virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	UE_API virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	UE_API virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	UE_API virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	bool bOutOfHealth = false;
	float HealthBeforeAttributeChange = 0.0f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	/** 本次结算的伤害量，消费后归零，不复制。 */
	UPROPERTY(BlueprintReadOnly, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MetaDamage;

	/** 本次结算的治疗量，消费后归零，不复制。 */
	UPROPERTY(BlueprintReadOnly, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MetaHealing;
};

#undef UE_API
