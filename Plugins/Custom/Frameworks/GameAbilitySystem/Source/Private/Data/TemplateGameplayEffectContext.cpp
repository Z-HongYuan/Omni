// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/TemplateGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TemplateGameplayEffectContext)

UScriptStruct* FTemplateGameplayEffectContext::GetScriptStruct() const
{
	return FTemplateGameplayEffectContext::StaticStruct();
}

FGameplayEffectContext* FTemplateGameplayEffectContext::Duplicate() const
{
	FTemplateGameplayEffectContext* NewContext = new FTemplateGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// 深拷贝
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FTemplateGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// 再序列化你自定义的字段
	// Ar << MyCustomField;

	return true;
}

FTemplateGameplayEffectContext* FTemplateGameplayEffectContext::ExtractEffectContext(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* Base = Handle.Get();
	if (Base && Base->GetScriptStruct()->IsChildOf(FTemplateGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FTemplateGameplayEffectContext*>(Base);
	}
	return nullptr;
}
