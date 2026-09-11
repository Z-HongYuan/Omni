// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/CustomTaggedActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomTaggedActor)

ACustomTaggedActor::ACustomTaggedActor()
{
}

void ACustomTaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(StaticGameplayTags);
}

#if WITH_EDITOR
bool ACustomTaggedActor::CanEditChange(const FProperty* InProperty) const
{
	// 关闭编辑器的原生标签
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AActor, Tags)) return false;
	return Super::CanEditChange(InProperty);
}
#endif
