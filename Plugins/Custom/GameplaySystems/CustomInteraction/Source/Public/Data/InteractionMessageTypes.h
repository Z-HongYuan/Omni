// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/ObjectPtr.h"
#include "InteractionMessageTypes.generated.h"

class AActor;

/** 供消息路由传递交互发起方与持续时间。 */
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
