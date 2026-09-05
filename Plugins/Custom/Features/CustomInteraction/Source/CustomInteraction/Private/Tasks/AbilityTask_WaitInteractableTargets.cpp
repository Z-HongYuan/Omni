// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Tasks/AbilityTask_WaitInteractableTargets.h"

#include "AbilitySystemComponent.h"
#include "Core/InteractableTargetInterface.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitInteractableTargets)

void UAbilityTask_WaitInteractableTargets::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	// 只获取第一个的交互对象
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

void UAbilityTask_WaitInteractableTargets::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch) const
{
	if (!Ability) // 仅权威中执行
	{
		return;
	}

	//@TODO: 机器人怎么办,他没有控制器?
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// 转换为角度并使用原始俯仰
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UAbilityTask_WaitInteractableTargets::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0) // 如果失败，我们会朝远离中心的方向，但我们可能在球体内部，能找到一个好的出口点。
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera; // 减法而不是加法会得到另一个交点
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection); // 凸轮瞄准点与射程球体截获
			return true;
		}
	}
	return false;
}

void UAbilityTask_WaitInteractableTargets::UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTargetInterface>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (const TScriptInterface<IInteractableTargetInterface>& InteractiveTarget : InteractableTargets)
	{
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, TempOptions);
		InteractiveTarget->GatherInteractionOptions(InteractQuery, InteractionBuilder);

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;

			// 如果有手柄和目标能力系统，我们就是在目标上触发该技能。
			if (Option.TargetAbilitySystem && Option.TargetInteractionAbilityHandle.IsValid())
			{
				// 寻找 spec
				InteractionAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
			}
			// 如果有互动能力，那我们是在自己身上激活它。
			else if (Option.InteractionAbilityToGrant)
			{
				// 寻找 spec
				InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.InteractionAbilityToGrant);

				if (InteractionAbilitySpec)
				{
					// 更新交互选项
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
				}
			}

			if (InteractionAbilitySpec)
			{
				// 过滤掉目前无法激活的选项。
				if (InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	bool bOptionsChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();

		for (int OptionIndex = 0; OptionIndex < NewOptions.Num(); OptionIndex++)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
			{
				bOptionsChanged = true;
				break;
			}
		}
	}
	else
	{
		bOptionsChanged = true;
	}

	if (bOptionsChanged)
	{
		CurrentOptions = NewOptions;
		InteractableObjectsChanged.Broadcast(CurrentOptions);
	}
}
