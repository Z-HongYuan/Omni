// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExperienceGameState.h"

#include "Components/ExperienceManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceGameState)

AExperienceGameState::AExperienceGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UExperienceManagerComponent>(this, TEXT("ExperienceManagerComponent"));
}
