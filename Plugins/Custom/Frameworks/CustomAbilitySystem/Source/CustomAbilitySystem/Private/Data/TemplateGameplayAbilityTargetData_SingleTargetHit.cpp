// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/TemplateGameplayAbilityTargetData_SingleTargetHit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TemplateGameplayAbilityTargetData_SingleTargetHit)

void FTemplateGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// // Add game-specific data
	// if (FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Context))
	// {
	// 	TypedContext->CartridgeID = CartridgeID;
	// }
}

bool FTemplateGameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	// Ar << CartridgeID;

	return true;
}

UScriptStruct* FTemplateGameplayAbilityTargetData_SingleTargetHit::GetScriptStruct() const
{
	return FTemplateGameplayAbilityTargetData_SingleTargetHit::StaticStruct();
}
