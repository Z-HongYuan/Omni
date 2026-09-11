// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/ExtGameplayAbilityTargetData_SingleTargetHit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtGameplayAbilityTargetData_SingleTargetHit)

void FExtGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// // Add game-specific data
	// if (FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Context))
	// {
	// 	TypedContext->CartridgeID = CartridgeID;
	// }
}

bool FExtGameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	// Ar << CartridgeID;

	return true;
}

UScriptStruct* FExtGameplayAbilityTargetData_SingleTargetHit::GetScriptStruct() const
{
	return FExtGameplayAbilityTargetData_SingleTargetHit::StaticStruct();
}
