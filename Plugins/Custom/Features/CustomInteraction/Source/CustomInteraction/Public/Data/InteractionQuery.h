// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InteractionQuery.generated.h"

/** 交互时的请求 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInteractionQuery
{
	GENERATED_BODY()

public:
	/** 请求的 Pawn */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** 允许我们指定一个控制器——不需要匹配请求头像的所有者。 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;

	/** 一个通用的UObject，用于插入交互所需的额外数据 */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> OptionalObjectData;
};
