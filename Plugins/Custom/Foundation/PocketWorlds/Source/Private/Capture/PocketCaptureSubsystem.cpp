// Copyright Epic Games, Inc. All Rights Reserved.

#include "Capture/PocketCaptureSubsystem.h"

#include "Components/PrimitiveComponent.h"
#include "Capture/PocketCapture.h"
#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PocketCaptureSubsystem)

class FSubsystemCollectionBase;

void UPocketCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick));
}

void UPocketCaptureSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	for (int32 RendererIndex = 0; RendererIndex < ThumbnailRenderers.Num(); RendererIndex++)
	{
		if (UPocketCapture* Renderer = ThumbnailRenderers[RendererIndex].Get())
		{
			Renderer->Deinitialize();
		}
	}

	ThumbnailRenderers.Reset();

	// 子系统退出时也恢复所有尚未归还的流送标记。
	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, bool>& Entry : OriginalMipStreamingStates)
	{
		if (UPrimitiveComponent* Component = Entry.Key.Get())
		{
			Component->bForceMipStreaming = Entry.Value;
		}
	}
	OriginalMipStreamingStates.Reset();
	StreamNextFrame.Reset();
	StreamedLastFrameButNotNext.Reset();
	TickHandle.Reset();

	Super::Deinitialize();
}

UPocketCapture* UPocketCaptureSubsystem::CreateThumbnailRenderer(TSubclassOf<UPocketCapture> ThumbnailRendererClass)
{
	if (!IsValid(ThumbnailRendererClass.Get()) || ThumbnailRendererClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) || !IsValid(GetWorld()))
	{
		return nullptr;
	}

	UPocketCapture* Renderer = NewObject<UPocketCapture>(this, ThumbnailRendererClass);

	int32 RendererEmptyIndex = ThumbnailRenderers.IndexOfByKey(nullptr);
	if (RendererEmptyIndex == INDEX_NONE)
	{
		RendererEmptyIndex = ThumbnailRenderers.Add(Renderer);
	}
	else
	{
		ThumbnailRenderers[RendererEmptyIndex] = Renderer;
	}

	Renderer->Initialize(GetWorld(), RendererEmptyIndex);

	return Renderer;
}

void UPocketCaptureSubsystem::DestroyThumbnailRenderer(UPocketCapture* ThumbnailRenderer)
{
	if (ThumbnailRenderer)
	{
		const int32 ThumbnailIndex = ThumbnailRenderers.IndexOfByKey(ThumbnailRenderer);
		if (ThumbnailIndex != INDEX_NONE)
		{
			ThumbnailRenderers[ThumbnailIndex] = nullptr;
			ThumbnailRenderer->Deinitialize();
		}
	}
}

void UPocketCaptureSubsystem::StreamThisFrame(TArray<UPrimitiveComponent*>& PrimitiveComponents)
{
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		// 连续拍摄同一组件时只记录一次原值，避免把临时设置的 true 当成原值。
		const TWeakObjectPtr<UPrimitiveComponent> ComponentPtr(PrimitiveComponent);
		if (!OriginalMipStreamingStates.Contains(ComponentPtr))
		{
			OriginalMipStreamingStates.Add(ComponentPtr, PrimitiveComponent->bForceMipStreaming != 0);
		}
		PrimitiveComponent->bForceMipStreaming = true;
		StreamedLastFrameButNotNext.Remove(ComponentPtr);
		StreamNextFrame.AddUnique(ComponentPtr);
	}
}

bool UPocketCaptureSubsystem::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_URealTimeThumbnailSubsystem_Tick);

	for (TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent : StreamedLastFrameButNotNext)
	{
		if (const bool* OriginalValue = OriginalMipStreamingStates.Find(PrimitiveComponent))
		{
			if (UPrimitiveComponent* Component = PrimitiveComponent.Get())
			{
				Component->bForceMipStreaming = *OriginalValue;
			}
			OriginalMipStreamingStates.Remove(PrimitiveComponent);
		}
	}

	StreamedLastFrameButNotNext = MoveTemp(StreamNextFrame);

	return true;
}
