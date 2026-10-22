// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InteractionQuery.generated.h"

/** 收集交互选项时使用的请求上下文。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInteractionQuery
{
	GENERATED_BODY()

public:
	/** 发起交互的 Avatar Actor。 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** 请求使用的控制器，可与 RequestingAvatar 的 Owner 不同。 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;

	/** 交互查询所需的可选附加数据；此弱引用不延长对象生命周期。 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> OptionalObjectData;
};
