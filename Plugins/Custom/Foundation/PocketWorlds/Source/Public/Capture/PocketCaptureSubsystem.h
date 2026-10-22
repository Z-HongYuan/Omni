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
 * 创建和注销口袋世界拍摄器，并临时接管参与拍摄组件的纹理流送标记。
 * 组件停止参与拍摄后通过 Ticker 恢复原标记；子系统只弱引用跟踪拍摄器。
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

	/**
	 * 创建指定类型的拍摄器；类型无效时返回 nullptr。
	 * 调用方需持有强引用，并在不再使用时调用 DestroyThumbnailRenderer。
	 */
	UFUNCTION(BlueprintCallable, Category=Pocket, meta = (DeterminesOutputType = "PocketCaptureClass"))
	UE_API UPocketCapture* CreateThumbnailRenderer(TSubclassOf<UPocketCapture> PocketCaptureClass);

	/** 注销拍摄器组件并释放其跟踪槽位；不直接销毁 UObject。 */
	UFUNCTION(BlueprintCallable, Category=Pocket)
	UE_API void DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer);

	/** 标记组件参与本帧拍摄，并临时启用其强制纹理流送标记。 */
	UE_API void StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents);

protected:
	/** 每帧回调：为不再需要拍摄的组件恢复接管前的强制流送标记。 */
	UE_API bool Tick(float DeltaTime);

	/** 下一帧需要强制流送的组件。 */
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamNextFrame;
	/** 等待后续 Tick 恢复流送标记的组件；再次参与拍摄时会从此列表移除。 */
	TArray<TWeakObjectPtr<UPrimitiveComponent>> StreamedLastFrameButNotNext;

private:
	/** 仅弱引用跟踪拍摄器，不负责保活；已销毁的槽位供后续创建复用。 */
	TArray<TWeakObjectPtr<UPocketCapture>> ThumbnailRenderers;

	/** 首次接管组件时的标记原值，停止拍摄或反初始化时恢复。 */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> OriginalMipStreamingStates;

	/** 核心 Ticker 句柄，用于 Deinitialize 时反注册。 */
	FTSTicker::FDelegateHandle TickHandle;
};

#undef UE_API
