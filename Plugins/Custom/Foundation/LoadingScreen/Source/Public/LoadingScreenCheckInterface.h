// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "LoadingScreenCheckInterface.generated.h"

#define UE_API LOADINGSCREEN_API

/** 供对象报告是否仍需显示加载画面的查询接口。 */
UINTERFACE(MinimalAPI, BlueprintType)
class ULoadingScreenCheckInterface : public UInterface
{
	GENERATED_BODY()
};

class ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	/** 对象实现此接口时查询其加载请求；无对象或未实现接口时返回 false。 */
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

	/** 返回是否仍需显示加载画面，并在需要显示时填写 OutReason。 */
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const { return false; }
};

#undef UE_API
