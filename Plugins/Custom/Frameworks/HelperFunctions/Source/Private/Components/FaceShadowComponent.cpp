// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/FaceShadowComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaceShadowComponent)

UFaceShadowComponent::UFaceShadowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.033f;
	SetIsReplicatedByDefault(false);
}

void UFaceShadowComponent::BeginPlay()
{
	Super::BeginPlay();

	RefreshMeshComponentRef();
}

void UFaceShadowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TargetMeshComponent.IsValid() && TargetMeshComponent->WasRecentlyRendered())
	{
		const FVector Head = TargetMeshComponent->GetSocketLocation(HeadSlotName);
		const FVector CurrentForwardDirection = (TargetMeshComponent->GetSocketLocation(ForwardSlotName) - Head).GetSafeNormal();
		const FVector CurrentRightDirection = (TargetMeshComponent->GetSocketLocation(RightSlotName) - Head).GetSafeNormal();

		if (!(CurrentForwardDirection == PreviousForwardDirection && CurrentRightDirection == PreviousRightDirection))
		{
			PreviousForwardDirection = CurrentForwardDirection;
			PreviousRightDirection = CurrentRightDirection;
			TargetMeshComponent->SetCustomPrimitiveDataVector3(FaceForwardOffsetIndex, CurrentForwardDirection);
			TargetMeshComponent->SetCustomPrimitiveDataVector3(FaceRightOffsetIndex, CurrentRightDirection);
		}
	}
}

void UFaceShadowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TargetMeshComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UFaceShadowComponent::RefreshMeshComponentRef()
{
	TargetMeshComponent = GetOwner()->FindComponentByTag<USkeletalMeshComponent>(SkeletalMeshComponentTag);
}
