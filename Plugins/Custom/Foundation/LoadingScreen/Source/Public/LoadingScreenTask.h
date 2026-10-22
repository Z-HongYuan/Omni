// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "LoadingScreenCheckInterface.h"
#include "UObject/Object.h"
#include "LoadingScreenTask.generated.h"

#define UE_API LOADINGSCREEN_API

/** 供蓝图创建和撤销的加载画面请求。 */
UCLASS(MinimalAPI, BlueprintType)
class ULoadingScreenTask : public UObject, public ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	ULoadingScreenTask() = default;

	/** 创建并注册加载请求；调用方必须持有返回任务的强引用，并通过 Unregister 撤销请求。 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen, meta=(WorldContext = "WorldContextObject"))
	static UE_API ULoadingScreenTask* CreateLoadingScreenTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

	/** 从加载管理器注销本任务，撤销其显示请求。 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	UE_API void Unregister();

	/** 更新本任务要求显示加载画面的诊断原因。 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	UE_API void SetShowLoadingScreenReason(const FString& InReason);

	// ILoadingScreenCheckInterface 接口
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	// ILoadingScreenCheckInterface 接口结束

private:
	FString Reason = FString();
};

#undef UE_API
