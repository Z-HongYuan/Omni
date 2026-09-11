// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ExperienceReady.generated.h"

#define UE_API EXPERIENCESYSTEM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

/**
 * 等待体验加载完成的异步操作
 * 体验已经加载,会下帧调用
 * 体验未加载,会当体验加载完成后即时调用
 */
UCLASS(MinimalAPI)
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// 等待体验加载完成
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

protected:
	// 体验加载完成后调用的委托事件
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;

private:
	void ListenToExperienceLoading(const AGameStateBase* GameState);
};
#undef UE_API
