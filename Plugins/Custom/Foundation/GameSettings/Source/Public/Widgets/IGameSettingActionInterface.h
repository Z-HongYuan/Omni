// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "IGameSettingActionInterface.generated.h"

#define UE_API GAMESETTINGS_API

class UGameSetting;
class UObject;
struct FFrame;

UINTERFACE(MinimalAPI, meta = (BlueprintType))
class UGameSettingActionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IGameSettingActionInterface
{
	GENERATED_BODY()

public:
	/** 为指定设置执行 ActionTag 对应的操作，返回是否已处理。 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UE_API bool ExecuteActionForSetting(FGameplayTag ActionTag, UGameSetting* InSetting);
};

#undef UE_API
