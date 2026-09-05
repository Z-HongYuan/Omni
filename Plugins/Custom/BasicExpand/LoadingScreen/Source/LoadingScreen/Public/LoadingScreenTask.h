// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "LoadingScreenCheckInterface.h"
#include "UObject/Object.h"
#include "LoadingScreenTask.generated.h"

#define UE_API LOADINGSCREEN_API

/*
 * 提供蓝图可用的加载界面请求
 */
UCLASS(MinimalAPI, BlueprintType)
class ULoadingScreenTask : public UObject, public ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	ULoadingScreenTask() { ; }

	/*
	 * 请求加载界面显示
	 * 使用 Unregister 取消显示
	 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen, meta=(WorldContext = "WorldContextObject"))
	static UE_API ULoadingScreenTask* CreateLoadingScreenTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason);

	// 取消显示加载界面
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	UE_API void Unregister();

	// 设置加载界面显示的原因
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	UE_API void SetShowLoadingScreenReason(const FString& InReason);

	// Begin ILoadingScreenCheckInterface
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	// End ILoadingScreenCheckInterface

private:
	FString Reason = FString();
};

#undef UE_API
