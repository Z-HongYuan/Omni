// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/CustomGlobalAbilityManager.h"
#include "System/CustomAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomGlobalAbilityManager)

// FGlobalAppliedAbilityList
//
void FGlobalAppliedAbilityList::AddToASC(TSubclassOf<UGameplayAbility> Ability, UCustomAbilitySystemComponent* ASC)
{
	//先移除技能再添加技能
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
	Handles.Add(ASC, AbilitySpecHandle);
}

void FGlobalAppliedAbilityList::RemoveFromASC(UCustomAbilitySystemComponent* ASC)
{
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}

void FGlobalAppliedAbilityList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->ClearAbility(KVP.Value);
		}
	}
	Handles.Empty();
}

// FGlobalAppliedEffectList
//
void FGlobalAppliedEffectList::AddToASC(TSubclassOf<UGameplayEffect> Effect, UCustomAbilitySystemComponent* ASC)
{
	//先移除效果再添加效果
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, 1, ASC->MakeEffectContext());
	Handles.Add(ASC, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromASC(UCustomAbilitySystemComponent* ASC)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(ASC);
	}
}

void FGlobalAppliedEffectList::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
		}
	}
	Handles.Empty();
}

// UCustomGlobalAbilityManager
//
void UCustomGlobalAbilityManager::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	//如果技能有效且未被添加过
	if ((Ability.Get() != nullptr) && (!AppliedAbilities.Contains(Ability)))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities.Add(Ability);
		for (UCustomAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Ability, ASC);
		}
	}
}

void UCustomGlobalAbilityManager::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && (!AppliedEffects.Contains(Effect)))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects.Add(Effect);
		for (UCustomAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.AddToASC(Effect, ASC);
		}
	}
}

void UCustomGlobalAbilityManager::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && AppliedAbilities.Contains(Ability))
	{
		FGlobalAppliedAbilityList& Entry = AppliedAbilities[Ability];
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

void UCustomGlobalAbilityManager::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && AppliedEffects.Contains(Effect))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects[Effect];
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

void UCustomGlobalAbilityManager::RegisterASC(UCustomAbilitySystemComponent* ASC)
{
	check(ASC);
	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.AddToASC(Entry.Key, ASC);
	}

	RegisteredASCs.AddUnique(ASC);
}

void UCustomGlobalAbilityManager::UnregisterASC(UCustomAbilitySystemComponent* ASC)
{
	check(ASC);
	for (auto& Entry : AppliedAbilities)
	{
		Entry.Value.RemoveFromASC(ASC);
	}
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.RemoveFromASC(ASC);
	}

	RegisteredASCs.Remove(ASC);
}
