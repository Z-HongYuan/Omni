// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ExperienceReady.generated.h"

#define UE_API EXPERIENCESYSTEM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExpReadyAsyncDelegate);

/**
 * 等待体验就绪的蓝图异步操作，由 GameInstance 持有至完成。
 * 体验已就绪时延后一帧通知，否则在加载完成回调中通知，随后释放操作。
 */
UCLASS(MinimalAPI)
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 创建体验就绪监听操作；上下文无法解析为世界时返回空指针。 */
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

protected:
	/** 体验就绪后广播一次，随后释放异步操作。 */
	UPROPERTY(BlueprintAssignable)
	FExpReadyAsyncDelegate OnReady;

private:
	void ListenToExperienceLoading(const AGameStateBase* GameState);
};
#undef UE_API
