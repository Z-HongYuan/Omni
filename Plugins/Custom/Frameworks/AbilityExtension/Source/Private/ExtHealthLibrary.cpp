// Copyright © 2026 张鸿源. All Rights Reserved.

#include "ExtHealthLibrary.h"

#include "AbilitySystemComponent.h"
#include "ExtAbilitySystemSettings.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtHealthLibrary)

namespace
{
	bool ApplyConfiguredEffect(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount,
	                           TSubclassOf<UGameplayEffect> Effect, FGameplayTag Tag, const FHitResult* Hit)
	{
		if (!Source || !Target || !Source->IsOwnerActorAuthoritative() || !Target->IsOwnerActorAuthoritative()) return false;
		if (!FMath::IsFinite(Amount) || Amount <= 0.0f || Source->GetWorld() != Target->GetWorld()) return false;
		if (!Effect || !Tag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Health effect or SetByCaller tag is missing in Ability Extension Settings."));
			return false;
		}
		FGameplayEffectContextHandle Context = Source->MakeEffectContext();
		if (Hit) Context.AddHitResult(*Hit);
		FGameplayEffectSpecHandle Spec = Source->MakeOutgoingSpec(Effect, 1.0f, Context);
		if (!Spec.IsValid()) return false;
		Spec.Data->SetSetByCallerMagnitude(Tag, Amount);
		return Source->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target).WasSuccessfullyApplied();
	}
}

bool UExtHealthLibrary::ApplyDamage(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount, const FHitResult& Hit)
{
	const UExtAbilitySystemSettings* Settings = GetDefault<UExtAbilitySystemSettings>();
	return ApplyConfiguredEffect(Source, Target, Amount, Settings->DamageGameplayEffect_SetByCaller, Settings->DamageSetByCallerTag, &Hit);
}

bool UExtHealthLibrary::ApplyHealing(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount)
{
	const UExtAbilitySystemSettings* Settings = GetDefault<UExtAbilitySystemSettings>();
	return ApplyConfiguredEffect(Source, Target, Amount, Settings->HealGameplayEffect_SetByCaller, Settings->HealSetByCallerTag, nullptr);
}
