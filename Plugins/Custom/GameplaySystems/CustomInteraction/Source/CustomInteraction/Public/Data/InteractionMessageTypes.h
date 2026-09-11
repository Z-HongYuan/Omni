// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/ObjectPtr.h"
#include "InteractionMessageTypes.generated.h"

class AActor;

/*
 * 用于MessageRouter使用的互动消息结构体
 */
USTRUCT(BlueprintType)
struct FInteractionDurationMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float Duration = 0;
};
