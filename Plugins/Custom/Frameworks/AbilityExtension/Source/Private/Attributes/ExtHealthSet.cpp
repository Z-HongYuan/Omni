// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Attributes/ExtHealthSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtHealthSet)

UExtHealthSet::UExtHealthSet()
	: Health(100.0f), MaxHealth(100.0f), MetaDamage(0.0f), MetaHealing(0.0f)
{
}

void UExtHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UExtHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UExtHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UExtHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UExtHealthSet, Health, OldValue);
}

void UExtHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UExtHealthSet, MaxHealth, OldValue);
}

void UExtHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UExtHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UExtHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 降低上限时压低当前生命；提高上限不自动治疗。
	if (Attribute == GetMaxHealthAttribute() && GetHealth() > NewValue)
	{
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}

void UExtHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 处理Mate伤害
	if (Data.EvaluatedData.Attribute == GetMetaDamageAttribute())
	{
		const float DamageAmount = FMath::Max(GetMetaDamage(), 0.0f);
		SetMetaDamage(0.0f);
		SetHealth(FMath::Clamp(GetHealth() - DamageAmount, 0.0f, GetMaxHealth()));
	}

	// 处理Mate治疗
	if (Data.EvaluatedData.Attribute == GetMetaHealingAttribute())
	{
		const float HealingAmount = FMath::Max(GetMetaHealing(), 0.0f);
		SetMetaHealing(0.0f);
		SetHealth(FMath::Clamp(GetHealth() + HealingAmount, 0.0f, GetMaxHealth()));
	}

	// 处理原生健康
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

void UExtHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
