// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Subsystems/WorldSubsystem.h"

#include "PocketCaptureSubsystem.generated.h"

#define UE_API POCKETWORLDS_API

template <typename T>
class TSubclassOf;

class FSubsystemCollectionBase;
class UObject;
class UPocketCapture;
class UPrimitiveComponent;
struct FFrame;

/**
 * 口袋世界拍摄器的管理子系统（WorldSubsystem）。
 *
 * 负责创建与销毁 UPocketCapture，并通过每帧 Ticker 维护贴图流送：
 * 只有这一帧真正要被拍摄的组件才强制流送，避免预览用的高分辨率贴图长期占用内存。
 */
UCLASS(MinimalAPI, BlueprintType)
class UPocketCaptureSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem 接口
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	//~USubsystem 接口结束

	// 创建一个指定类型的拍摄器，返回类型由 PocketCaptureClass 决定。无效类型返回 nullptr。调用方需持有强引用，不再使用时调用 DestroyThumbnailRenderer。
	UFUNCTION(BlueprintCallable, Category=Pocket, meta = (DeterminesOutputType = "PocketCaptureClass"))
	UE_API UPocketCapture* CreateThumbnailRenderer(TSubclassOf<UPocketCapture> PocketCaptureClass);

	// 销毁拍摄器，并把它在数组中的槽位置空以供后续复用。
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer);

	// 通知子系统：这些组件在本帧要参与拍摄，请立刻把它们的贴图强制流送进来。
	UE_API void StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents);

protected:
	// 每帧回调：为不再需要拍摄的组件恢复接管前的强制流送标记。
	UE_API bool Tick(float DeltaTime);

	// 下一帧需要强制流送的组件。
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamNextFrame;
	// 上一帧流送过、但下一帧不再需要的组件，下一次 Tick 会解除它们的强制流送。
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamedLastFrameButNotNext;

private:
	// 仅弱引用跟踪拍摄器，不负责保活；已销毁的槽位供后续创建复用。
	TArray<TWeakObjectPtr<UPocketCapture>> ThumbnailRenderers;

	// 首次接管组件时的标记原值，停止拍摄或反初始化时恢复。
	TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> OriginalMipStreamingStates;

	// 核心 Ticker 句柄，用于 Deinitialize 时反注册。
	FTSTicker::FDelegateHandle TickHandle;
};

#undef UE_API
