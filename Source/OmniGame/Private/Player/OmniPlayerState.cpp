// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Player/OmniPlayerState.h"

#include "Attributes/ExtHealthSet.h"
#include "Data/ExpPawnData.h"
#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniPlayerState)

AOmniPlayerState::AOmniPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HealthSet = CreateDefaultSubobject<UExtHealthSet>(TEXT("HealthSet"));
}

void AOmniPlayerState::OnExperienceLoaded(const UExpDefinition* CurrentExperience)
{
	Super::OnExperienceLoaded(CurrentExperience);

	UE_LOG(LogOmniGame, Log, TEXT("OmniPlayerState OnExperienceLoaded: %s (%s), PawnData: %s"),
	       *GetPathName(), *GetClass()->GetPathName(), *GetPathNameSafe(GetPawnData<UExpPawnData>()));
}
