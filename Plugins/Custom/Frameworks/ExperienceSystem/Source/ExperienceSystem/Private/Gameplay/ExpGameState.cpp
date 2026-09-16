// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpGameState.h"

#include "Components/ExpManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpGameState)

AExpGameState::AExpGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ExperienceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UExpManagerComponent>(this, TEXT("ExperienceManagerComponent"));
}
