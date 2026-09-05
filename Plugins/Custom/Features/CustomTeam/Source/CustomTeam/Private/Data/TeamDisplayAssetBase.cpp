// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/TeamDisplayAssetBase.h"

#include "Core/TeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamDisplayAssetBase)

#if WITH_EDITOR
void UTeamDisplayAssetBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (UTeamSubsystem* TeamSubsystem : TObjectRange<UTeamSubsystem>())
	{
		TeamSubsystem->NotifyTeamDisplayAssetModified(this);
	}
}
#endif
