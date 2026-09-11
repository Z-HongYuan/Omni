// Copyright © 2026 张鸿源. All Rights Reserved.


#include "IndicatorDescriptorDataObj.h"
#include "IndicatorManagerComponent.h"
#include "SceneView.h"
#include "Engine/LocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorDescriptorDataObj)

bool UIndicatorDescriptorDataObj::Project(const UIndicatorDescriptorDataObj& IndicatorDescriptor, const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize, FVector& OutScreenPositionWithDepth)
{
	if (USceneComponent* Component = IndicatorDescriptor.IndicatorDescriptorParameter.SceneComponent)
	{
		TOptional<FVector> WorldLocation;
		if (IndicatorDescriptor.IndicatorDescriptorParameter.ComponentSocketName != NAME_None)
		{
			WorldLocation = Component->GetSocketTransform(IndicatorDescriptor.IndicatorDescriptorParameter.ComponentSocketName).GetLocation();
		}
		else
		{
			WorldLocation = Component->GetComponentLocation();
		}

		const FVector ProjectWorldLocation = WorldLocation.GetValue() + IndicatorDescriptor.IndicatorDescriptorParameter.WorldPositionOffset;
		const EActorCanvasProjectionMode ProjectionMode = IndicatorDescriptor.IndicatorDescriptorParameter.ProjectionMode;

		switch (ProjectionMode)
		{
		case EActorCanvasProjectionMode::ComponentPoint:
			{
				if (WorldLocation.IsSet())
				{
					FVector2D OutScreenSpacePosition;
					const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectWorldLocation, OutScreenSpacePosition, &ScreenSize);

					OutScreenSpacePosition.X += IndicatorDescriptor.IndicatorDescriptorParameter.ScreenSpaceOffset.X * (bInFrontOfCamera ? 1 : -1);
					OutScreenSpacePosition.Y += IndicatorDescriptor.IndicatorDescriptorParameter.ScreenSpaceOffset.Y;

					if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
					{
						const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
						OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
					}

					OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation));

					return true;
				}

				return false;
			}
		case EActorCanvasProjectionMode::ComponentScreenBoundingBox:
		case EActorCanvasProjectionMode::ActorScreenBoundingBox:
			{
				FBox IndicatorBox;
				if (ProjectionMode == EActorCanvasProjectionMode::ActorScreenBoundingBox)
				{
					IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
				}
				else
				{
					IndicatorBox = Component->Bounds.GetBox();
				}

				FVector2D LL, UR;
				const bool bInFrontOfCamera = ULocalPlayer::GetPixelBoundingBox(InProjectionData, IndicatorBox, LL, UR, &ScreenSize);

				const FVector& BoundingBoxAnchor = IndicatorDescriptor.IndicatorDescriptorParameter.BoundingBoxAnchor;
				const FVector2D& ScreenSpaceOffset = IndicatorDescriptor.IndicatorDescriptorParameter.ScreenSpaceOffset;

				FVector ScreenPositionWithDepth;
				ScreenPositionWithDepth.X = FMath::Lerp(LL.X, UR.X, BoundingBoxAnchor.X) + ScreenSpaceOffset.X * (bInFrontOfCamera ? 1 : -1);
				ScreenPositionWithDepth.Y = FMath::Lerp(LL.Y, UR.Y, BoundingBoxAnchor.Y) + ScreenSpaceOffset.Y;
				ScreenPositionWithDepth.Z = FVector::Dist(InProjectionData.ViewOrigin, ProjectWorldLocation);

				const FVector2f ScreenSpacePosition = FVector2f(FVector2D(ScreenPositionWithDepth));
				if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside(ScreenSpacePosition))
				{
					const FVector2f CenterToPosition = (ScreenSpacePosition - (ScreenSize / 2)).GetSafeNormal();
					const FVector2f ScreenPositionFromBehind = (ScreenSize / 2) + CenterToPosition * ScreenSize;
					ScreenPositionWithDepth.X = ScreenPositionFromBehind.X;
					ScreenPositionWithDepth.Y = ScreenPositionFromBehind.Y;
				}

				OutScreenPositionWithDepth = ScreenPositionWithDepth;
				return true;
			}
		case EActorCanvasProjectionMode::ActorBoundingBox:
		case EActorCanvasProjectionMode::ComponentBoundingBox:
			{
				FBox IndicatorBox;
				if (ProjectionMode == EActorCanvasProjectionMode::ActorBoundingBox)
				{
					IndicatorBox = Component->GetOwner()->GetComponentsBoundingBox();
				}
				else
				{
					IndicatorBox = Component->Bounds.GetBox();
				}

				const FVector ProjectBoxPoint = IndicatorBox.GetCenter() + (IndicatorBox.GetSize() * (IndicatorDescriptor.IndicatorDescriptorParameter.BoundingBoxAnchor - FVector(0.5)));

				FVector2D OutScreenSpacePosition;
				const bool bInFrontOfCamera = ULocalPlayer::GetPixelPoint(InProjectionData, ProjectBoxPoint, OutScreenSpacePosition, &ScreenSize);
				OutScreenSpacePosition.X += IndicatorDescriptor.IndicatorDescriptorParameter.ScreenSpaceOffset.X * (bInFrontOfCamera ? 1 : -1);
				OutScreenSpacePosition.Y += IndicatorDescriptor.IndicatorDescriptorParameter.ScreenSpaceOffset.Y;

				if (!bInFrontOfCamera && FBox2f(FVector2f::Zero(), ScreenSize).IsInside((FVector2f)OutScreenSpacePosition))
				{
					const FVector2f CenterToPosition = (FVector2f(OutScreenSpacePosition) - (ScreenSize / 2)).GetSafeNormal();
					OutScreenSpacePosition = FVector2D((ScreenSize / 2) + CenterToPosition * ScreenSize);
				}

				OutScreenPositionWithDepth = FVector(OutScreenSpacePosition.X, OutScreenSpacePosition.Y, FVector::Dist(InProjectionData.ViewOrigin, ProjectBoxPoint));

				return true;
			}
		}
	}

	return false;
}

void UIndicatorDescriptorDataObj::SetIndicatorManagerComponent(UIndicatorManagerComponent* InManager)
{
	// 确保没人设置过这个
	if (ensure(IndicatorDescriptorParameter.ManagerPtr.IsExplicitlyNull()))
	{
		IndicatorDescriptorParameter.ManagerPtr = InManager;
	}
}

void UIndicatorDescriptorDataObj::UnregisterIndicator()
{
	if (UIndicatorManagerComponent* Manager = IndicatorDescriptorParameter.ManagerPtr.Get())
	{
		Manager->RemoveIndicator(this);
	}
}
