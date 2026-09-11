// Copyright Epic Games, Inc. All Rights Reserved.

#include "Capture/PocketCapture.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ScopeExit.h"
#include "RHIGlobals.h"
#include "UObject/StrongObjectPtr.h"
#include "Capture/PocketCaptureSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PocketCapture)

class UWorld;

void UPocketCapture::Initialize(UWorld* InWorld, int32 InRendererIndex)
{
	PrivateWorld = InWorld;
	RendererIndex = InRendererIndex;

	CaptureComponent = NewObject<USceneCaptureComponent2D>(this, "Thumbnail_Capture_Component");
	CaptureComponent->RegisterComponentWithWorld(InWorld);
	CaptureComponent->bConsiderUnrenderedOpaquePixelAsFullyTranslucent = true;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->bAlwaysPersistRenderingState = true;
}

void UPocketCapture::Deinitialize()
{
	CaptureComponent->UnregisterComponent();
}

void UPocketCapture::BeginDestroy()
{
	Super::BeginDestroy();

	if (CaptureComponent)
	{
		CaptureComponent->UnregisterComponent();
		CaptureComponent = nullptr;
	}
}

void UPocketCapture::SetRenderTargetSize(int32 Width, int32 Height)
{
	const uint32 MaxDimension = GetMax2DTextureDimension();
	if (Width <= 0 || Height <= 0 || static_cast<uint32>(Width) > MaxDimension || static_cast<uint32>(Height) > MaxDimension)
	{
		return;
	}

	if (SurfaceWidth != Width || SurfaceHeight != Height)
	{
		SurfaceWidth = Width;
		SurfaceHeight = Height;

		if (DiffuseRT)
		{
			DiffuseRT->ResizeTarget(SurfaceWidth, SurfaceHeight);
		}

		if (AlphaMaskRT)
		{
			AlphaMaskRT->ResizeTarget(SurfaceWidth, SurfaceHeight);
		}

		if (EffectsRT)
		{
			EffectsRT->ResizeTarget(SurfaceWidth, SurfaceHeight);
		}
	}
}

UTextureRenderTarget2D* UPocketCapture::GetOrCreateDiffuseRenderTarget()
{
	if (DiffuseRT == nullptr)
	{
		DiffuseRT = NewObject<UTextureRenderTarget2D>(this, TEXT("ThumbnailRenderer_Diffuse"));
		DiffuseRT->RenderTargetFormat = RTF_RGBA8;
		DiffuseRT->InitAutoFormat(SurfaceWidth, SurfaceHeight);
		DiffuseRT->UpdateResourceImmediate(true);
	}

	return DiffuseRT;
}

UTextureRenderTarget2D* UPocketCapture::GetOrCreateAlphaMaskRenderTarget()
{
	if (AlphaMaskRT == nullptr)
	{
		AlphaMaskRT = NewObject<UTextureRenderTarget2D>(this, TEXT("ThumbnailRenderer_AlphaMask"));
		AlphaMaskRT->RenderTargetFormat = RTF_R8;
		AlphaMaskRT->InitAutoFormat(SurfaceWidth, SurfaceHeight);
		AlphaMaskRT->UpdateResourceImmediate(true);
	}

	return AlphaMaskRT;
}

UTextureRenderTarget2D* UPocketCapture::GetOrCreateEffectsRenderTarget()
{
	if (EffectsRT == nullptr)
	{
		EffectsRT = NewObject<UTextureRenderTarget2D>(this, TEXT("ThumbnailRenderer_Fx"));
		EffectsRT->RenderTargetFormat = RTF_R8;
		EffectsRT->InitAutoFormat(SurfaceWidth, SurfaceHeight);
		EffectsRT->UpdateResourceImmediate(true);
	}

	return EffectsRT;
}

void UPocketCapture::SetCaptureTarget(AActor* InCaptureTarget)
{
	CaptureTargetPtr = InCaptureTarget;

	OnCaptureTargetChanged(InCaptureTarget);
}

void UPocketCapture::SetAlphaMaskedActors(const TArray<AActor*>& InCaptureTargets)
{
	AlphaMaskActorPtrs.Reset();

	for (AActor* CaptureTarget : InCaptureTargets)
	{
		AlphaMaskActorPtrs.Add(CaptureTarget);
	}
}

UPocketCaptureSubsystem* UPocketCapture::GetThumbnailSystem() const
{
	return CastChecked<UPocketCaptureSubsystem>(GetOuter());
}

TArray<UPrimitiveComponent*> UPocketCapture::GatherPrimitivesForCapture(const TArray<AActor*>& InCaptureActors) const
{
	const bool bIncludeFromChildActors = true;
	TArray<UPrimitiveComponent*> PrimitiveComponents;

	for (AActor* CaptureActor : InCaptureActors)
	{
		if (!IsValid(CaptureActor))
		{
			continue;
		}

		TInlineComponentArray<UPrimitiveComponent*> ChildPrimitiveComponents;
		CaptureActor->GetComponents(ChildPrimitiveComponents, bIncludeFromChildActors);

		for (UPrimitiveComponent* ChildPrimitiveComponent : ChildPrimitiveComponents)
		{
			if (IsValid(ChildPrimitiveComponent) && !ChildPrimitiveComponent->bHiddenInGame)
			{
				PrimitiveComponents.AddUnique(ChildPrimitiveComponent);
			}
		}
	}

	return PrimitiveComponents;
}

bool UPocketCapture::CaptureScene(UTextureRenderTarget2D* InRenderTarget, const TArray<AActor*>& InCaptureActors, ESceneCaptureSource InCaptureSource, UMaterialInterface* OverrideMaterial)
{
	AActor* CaptureTarget = CaptureTargetPtr.Get();
	if (!IsValid(InRenderTarget) || !IsValid(CaptureTarget) || !IsValid(CaptureComponent) || !CaptureComponent->IsRegistered() || InCaptureActors.IsEmpty())
	{
		return false;
	}

	// 先检查相机并读取视角，失败时不触碰参与拍摄的材质。
	UCameraComponent* Camera = CaptureTarget->FindComponentByClass<UCameraComponent>();
	if (!IsValid(Camera))
	{
		return false;
	}

	FMinimalViewInfo CaptureView;
	Camera->GetCameraView(0, CaptureView);
	TArray<UPrimitiveComponent*> PrimitiveComponents = GatherPrimitivesForCapture(InCaptureActors);
	GetThumbnailSystem()->StreamThisFrame(PrimitiveComponents);

	struct FMaterialRestoreEntry
	{
		TWeakObjectPtr<UPrimitiveComponent> Component;
		int32 MaterialIndex = 0;
		TStrongObjectPtr<UMaterialInterface> OriginalMaterial;
	};
	TArray<FMaterialRestoreEntry> OriginalMaterials;

	// 覆盖材质只在本次拍摄中有效；任何退出路径都恢复原材质。
	ON_SCOPE_EXIT
	{
		for (const FMaterialRestoreEntry& Entry : OriginalMaterials)
		{
			if (UPrimitiveComponent* Component = Entry.Component.Get())
			{
				Component->SetMaterial(Entry.MaterialIndex, Entry.OriginalMaterial.Get());
			}
		}
	};

	if (OverrideMaterial)
	{
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			const int32 MaterialCount = PrimitiveComponent->GetNumMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; MaterialIndex++)
			{
				FMaterialRestoreEntry& Entry = OriginalMaterials.AddDefaulted_GetRef();
				Entry.Component = PrimitiveComponent;
				Entry.MaterialIndex = MaterialIndex;
				Entry.OriginalMaterial.Reset(PrimitiveComponent->GetMaterial(MaterialIndex));
				PrimitiveComponent->SetMaterial(MaterialIndex, OverrideMaterial);
			}
		}
	}

	CaptureComponent->ShowOnlyActors = InCaptureActors;
	CaptureComponent->TextureTarget = InRenderTarget;
	CaptureComponent->PostProcessSettings = Camera->PostProcessSettings;
	CaptureComponent->SetCameraView(CaptureView);

	CaptureComponent->ShowFlags.SetDepthOfField(false);
	CaptureComponent->ShowFlags.SetMotionBlur(false);
	CaptureComponent->ShowFlags.SetScreenPercentage(false);
	CaptureComponent->ShowFlags.SetScreenSpaceReflections(false);
	CaptureComponent->ShowFlags.SetDistanceFieldAO(false);

	CaptureComponent->ShowFlags.SetLensFlares(false);
	CaptureComponent->ShowFlags.SetOnScreenDebug(false);
	//CaptureComponent->ShowFlags.SetEyeAdaptation(false);
	CaptureComponent->ShowFlags.SetColorGrading(false);
	CaptureComponent->ShowFlags.SetCameraImperfections(false);
	CaptureComponent->ShowFlags.SetVignette(false);
	CaptureComponent->ShowFlags.SetGrain(false);
	CaptureComponent->ShowFlags.SetSeparateTranslucency(false);
	CaptureComponent->ShowFlags.SetTemporalAA(false);
	// 如果很少向它渲染，可能会触发资源重新分配 —— 暂时关闭
	CaptureComponent->ShowFlags.SetAmbientOcclusion(false);
	// 该特性需要 FScene 中的资源，一旦开启，每个临时场景都会重新分配这些资源
	CaptureComponent->ShowFlags.SetIndirectLightingCache(false);
	CaptureComponent->ShowFlags.SetLightShafts(false);
	CaptureComponent->ShowFlags.SetPostProcessMaterial(false);
	CaptureComponent->ShowFlags.SetHighResScreenshotMask(false);
	CaptureComponent->ShowFlags.SetHMDDistortion(false);
	CaptureComponent->ShowFlags.SetStereoRendering(false);
	CaptureComponent->ShowFlags.SetVolumetricFog(false);
	CaptureComponent->ShowFlags.SetVolumetricLightmap(false);
	CaptureComponent->ShowFlags.SetSkyLighting(false);

	CaptureComponent->CaptureSource = InCaptureSource;
	CaptureComponent->ProfilingEventName = TEXT("Pocket Capture");
	CaptureComponent->CaptureScene();
	return true;
}

void UPocketCapture::CaptureDiffuse()
{
	if (UTextureRenderTarget2D* RenderTarget = GetOrCreateDiffuseRenderTarget())
	{
		TArray<AActor*> CaptureActors;
		if (AActor* CaptureTarget = CaptureTargetPtr.Get())
		{
			CaptureTarget->GetAttachedActors(CaptureActors);
			CaptureActors.Add(CaptureTarget);
		}

		CaptureScene(RenderTarget, CaptureActors, ESceneCaptureSource::SCS_FinalColorLDR, nullptr);
	}
}

void UPocketCapture::CaptureAlphaMask()
{
	if (UTextureRenderTarget2D* RenderTarget = GetOrCreateAlphaMaskRenderTarget())
	{
		TArray<AActor*> CaptureActors;
		for (const TWeakObjectPtr<AActor>& AlphaMaskTargetPtr : AlphaMaskActorPtrs)
		{
			if (AActor* AlphaMaskTarget = AlphaMaskTargetPtr.Get())
			{
				CaptureActors.Add(AlphaMaskTarget);
			}
		}

		CaptureScene(RenderTarget, CaptureActors, ESceneCaptureSource::SCS_SceneColorHDR, AlphaMaskMaterial);
	}
}

void UPocketCapture::CaptureEffects()
{
	if (UTextureRenderTarget2D* RenderTarget = GetOrCreateEffectsRenderTarget())
	{
		ensure(false); //TODO
		TArray<AActor*> CaptureActors;
		CaptureScene(RenderTarget, CaptureActors, ESceneCaptureSource::SCS_SceneColorHDR, EffectMaskMaterial);
	}
}

void UPocketCapture::ReleaseResources()
{
	if (DiffuseRT)
	{
		DiffuseRT->ReleaseResource();
	}

	if (AlphaMaskRT)
	{
		AlphaMaskRT->ReleaseResource();
	}

	if (EffectsRT)
	{
		EffectsRT->ReleaseResource();
	}
}

void UPocketCapture::ReclaimResources()
{
	if (DiffuseRT)
	{
		DiffuseRT->UpdateResource();
	}

	if (AlphaMaskRT)
	{
		AlphaMaskRT->UpdateResource();
	}

	if (EffectsRT)
	{
		EffectsRT->UpdateResource();
	}
}

int32 UPocketCapture::GetRendererIndex() const
{
	return RendererIndex;
}
