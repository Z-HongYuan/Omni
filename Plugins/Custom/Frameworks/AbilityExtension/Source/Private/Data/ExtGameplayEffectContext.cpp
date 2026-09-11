// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/ExtGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtGameplayEffectContext)

UScriptStruct* FExtGameplayEffectContext::GetScriptStruct() const
{
	return FExtGameplayEffectContext::StaticStruct();
}

FGameplayEffectContext* FExtGameplayEffectContext::Duplicate() const
{
	FExtGameplayEffectContext* NewContext = new FExtGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// 深拷贝
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FExtGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// 再序列化你自定义的字段
	// Ar << MyExtField;

	return true;
}

FExtGameplayEffectContext* FExtGameplayEffectContext::ExtractEffectContext(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* Base = Handle.Get();
	if (Base && Base->GetScriptStruct()->IsChildOf(FExtGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FExtGameplayEffectContext*>(Base);
	}
	return nullptr;
}
