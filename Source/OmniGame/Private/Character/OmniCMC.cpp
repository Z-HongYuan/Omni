// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Character/OmniCMC.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCMC)

namespace OmniCharacter
{
	constexpr float GroundTraceDistance = 100000.0f;
}

UOmniCMC::UOmniCMC(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOmniCMC::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 移动后刷新地面快照，供后续消费者读取最终位置的数据。
	UpdateGroundInfo();
}

void UOmniCMC::TickCharacterPose(float DeltaTime)
{
	// Root Motion / 网络移动可能提前更新姿势，先刷新 GroundInfo 供这次动画读取。
	// 同帧移动后的位置和 CurrentFloor 仍可能改变，不能按帧跳过后续刷新。
	UpdateGroundInfo();

	Super::TickCharacterPose(DeltaTime);
}

void UOmniCMC::UpdateGroundInfo()
{
	check(IsInGameThread());

	if (!CharacterOwner || !UpdatedComponent || !GetWorld())
	{
		GroundInfo = FOmniCharacterGroundInfo();
		return;
	}

	if (MovementMode == MOVE_Walking)
	{
		GroundInfo.GroundHitResult = CurrentFloor.HitResult;
		GroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		// 沿世界向下检测，并扣除缩放后的半高，使距离以实际胶囊底部为起点。
		const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector TraceStart = UpdatedComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, OmniCharacter::GroundTraceDistance + CapsuleHalfHeight);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OmniCMC_UpdateGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParams;
		InitCollisionParams(QueryParams, ResponseParams);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,
			UpdatedComponent->GetCollisionObjectType(), QueryParams, ResponseParams);

		GroundInfo.GroundHitResult = HitResult;
		GroundInfo.GroundDistance = OmniCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
			GroundInfo.GroundDistance = 0.0f;
		else if (HitResult.bBlockingHit)
			GroundInfo.GroundDistance = FMath::Max(HitResult.Distance - CapsuleHalfHeight, 0.0f);
	}
}
