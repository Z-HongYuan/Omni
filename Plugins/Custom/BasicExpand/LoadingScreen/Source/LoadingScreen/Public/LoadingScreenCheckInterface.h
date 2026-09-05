// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "LoadingScreenCheckInterface.generated.h"

#define UE_API LOADINGSCREEN_API

/*
 * 检查和评估是否显示加载界面
 */
UINTERFACE(MinimalAPI, BlueprintType)
class ULoadingScreenCheckInterface : public UInterface
{
	GENERATED_BODY()
};

class ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	//检查对象是否实现了接口，如果实现了则检查是否应该显示加载画面
	static UE_API bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
	{
		if (TestObject == nullptr) return false;

		const ILoadingScreenCheckInterface* LoadObserver = Cast<ILoadingScreenCheckInterface>(TestObject);
		if (!LoadObserver) return false;

		FString ObserverReason;
		if (LoadObserver->ShouldShowLoadingScreen(ObserverReason))
		{
			if (ensureMsgf(!ObserverReason.IsEmpty(), TEXT("%s 没能设置显示加载画面的原因,因为为空"), *GetPathNameSafe(TestObject)))
			{
				OutReason = ObserverReason;
			}
			return true;
		}
		return false;
	}

	// 是否应该显示加载界面
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const { return false; }
};

#undef UE_API
