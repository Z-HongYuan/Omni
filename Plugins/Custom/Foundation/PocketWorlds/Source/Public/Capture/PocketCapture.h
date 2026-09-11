// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "PocketCapture.generated.h"

#define UE_API POCKETWORLDS_API

enum ESceneCaptureSource : int;

class UMaterialInterface;
class UPocketCaptureSubsystem;
class UPrimitiveComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UWorld;
struct FFrame;

/**
 * 口袋世界的离屏拍摄器基类。
 *
 * 负责把口袋小世界里指定的 Actor 用 SceneCaptureComponent2D 拍成 RenderTarget，供 UMG 直接采样显示。
 * 本类是抽象类，实际使用时应继承它，在子类里配置相机、遮罩材质并决定要拍摄哪些 Actor。
 * 实例由 UPocketCaptureSubsystem 创建并弱引用跟踪，调用方需用 UPROPERTY 等强引用保持存活。
 * Within 仅约束 Outer 类型；不再使用时调用 DestroyThumbnailRenderer 注销拍摄组件。
 */
UCLASS(MinimalAPI, Abstract, Within=PocketCaptureSubsystem, BlueprintType, Blueprintable)
class UPocketCapture : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketCapture() = default;

	// 初始化：创建并注册 SceneCaptureComponent2D。由子系统创建时自动调用，一般无需手动调用。
	UE_API virtual void Initialize(UWorld* InWorld, int32 RendererIndex);
	// 反初始化：注销 SceneCapture 组件。由子系统销毁时自动调用。
	UE_API virtual void Deinitialize();

	// 对象销毁时兜底注销 SceneCapture 组件。
	UE_API virtual void BeginDestroy() override;

	// 设置渲染目标分辨率；非正数或超过当前渲染接口上限时保留原尺寸。有效尺寸会同步应用到已有 RenderTarget。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void SetRenderTargetSize(int32 Width, int32 Height);

	// 获取彩色 RenderTarget（RGBA8），首次调用时创建。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API UTextureRenderTarget2D* GetOrCreateDiffuseRenderTarget();

	// 获取剪影遮罩 RenderTarget（R8），首次调用时创建。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API UTextureRenderTarget2D* GetOrCreateAlphaMaskRenderTarget();

	// 获取特效层 RenderTarget（R8），首次调用时创建。注意：特效层拍摄尚未实现，暂不可用。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API UTextureRenderTarget2D* GetOrCreateEffectsRenderTarget();

	// 设置拍摄目标。它身上的相机组件决定拍摄视角，它自身及挂载的子 Actor 会被拍进彩色图。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void SetCaptureTarget(AActor* InCaptureTarget);

	// 设置需要参与剪影遮罩拍摄的 Actor（会用遮罩材质覆盖后单独再拍一遍）。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void SetAlphaMaskedActors(const TArray<AActor*>& InCaptureTarget);

	// 执行一次彩色拍摄，结果写入 DiffuseRT。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void CaptureDiffuse();

	// 执行一次剪影遮罩拍摄，结果写入 AlphaMaskRT。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void CaptureAlphaMask();

	// 执行一次特效层拍摄，结果写入 EffectsRT。注意：当前仍是 TODO，调用会触发 ensure 失败。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void CaptureEffects();

	// 释放三张 RenderTarget 的渲染资源（如切后台、UI 隐藏时节省显存）。子类可重写以释放自己的资源。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API virtual void ReleaseResources();

	// 重新申请三张 RenderTarget 的渲染资源，与 ReleaseResources 配对使用。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API virtual void ReclaimResources();

	// 返回本拍摄器在子系统中占用的槽位索引。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API int32 GetRendererIndex() const;

protected:
	// 获取当前拍摄目标。目标可能已被销毁，因此可能返回 nullptr。
	AActor* GetCaptureTarget() const { return CaptureTargetPtr.Get(); }

	// 拍摄目标发生变化时的回调钩子。子类可重写以重建预览内容。
	virtual void OnCaptureTargetChanged(AActor* InCaptureTarget)
	{
	}

	// 执行一次场景拍摄：只渲染 InCaptureActors，可临时用 OverrideMaterial 覆盖它们的材质（拍完会还原）。
	UE_API bool CaptureScene(UTextureRenderTarget2D* InRenderTarget, const TArray<AActor*>& InCaptureActors, ESceneCaptureSource CaptureSource, UMaterialInterface* OverrideMaterial);

protected:
	// 收集这些 Actor（含子 Actor）上所有未被 bHiddenInGame 隐藏的 Primitive 组件。
	UE_API TArray<UPrimitiveComponent*> GatherPrimitivesForCapture(const TArray<AActor*>& InCaptureActors) const;

	// 获取持有本对象的拍摄子系统。
	UE_API UPocketCaptureSubsystem* GetThumbnailSystem() const;

protected:
	// 拍剪影遮罩时覆盖用的材质（通常是纯白材质）。
	UPROPERTY(EditDefaultsOnly, Category=Pocket)
	TObjectPtr<UMaterialInterface> AlphaMaskMaterial;

	// 拍特效层时覆盖用的材质（通常是纯黑材质）。
	UPROPERTY(EditDefaultsOnly, Category=Pocket)
	TObjectPtr<UMaterialInterface> EffectMaskMaterial;

protected:
	// 拍摄所在的世界，即口袋世界实例所在的世界。
	UPROPERTY(Transient)
	TObjectPtr<UWorld> PrivateWorld;

	// 本拍摄器在子系统中的槽位索引。
	UPROPERTY(Transient)
	int32 RendererIndex = INDEX_NONE;

	// 渲染目标的宽度（像素）。
	UPROPERTY(VisibleAnywhere)
	int32 SurfaceWidth = 1;

	// 渲染目标的高度（像素）。
	UPROPERTY(VisibleAnywhere)
	int32 SurfaceHeight = 1;

	// 彩色本体 RenderTarget。
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> DiffuseRT;

	// 剪影遮罩 RenderTarget。
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> AlphaMaskRT;

	// 特效层 RenderTarget。
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextureRenderTarget2D> EffectsRT;

	// 实际执行拍摄的 SceneCapture2D 组件。
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	// 当前拍摄目标（弱引用），主要提供拍摄视角。
	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<AActor> CaptureTargetPtr;

	// 需要参与剪影遮罩拍摄的 Actor 列表（弱引用）。
	UPROPERTY(VisibleAnywhere)
	TArray<TWeakObjectPtr<AActor>> AlphaMaskActorPtrs;
};

#undef UE_API
