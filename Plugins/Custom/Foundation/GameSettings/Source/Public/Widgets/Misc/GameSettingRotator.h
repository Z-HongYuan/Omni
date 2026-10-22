// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonRotator.h"

#include "GameSettingRotator.generated.h"

#define UE_API GAMESETTINGS_API

class UObject;

/** 设置项使用的选项轮换控件，将默认选项通知转发给蓝图。 */
UCLASS(MinimalAPI, Abstract, meta = (Category = "Settings", DisableNativeTick))
class UGameSettingRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UE_API UGameSettingRotator(const FObjectInitializer& Initializer);

	UE_API void SetDefaultOption(int32 DefaultOptionIndex);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Events, meta = (DisplayName = "On Default Option Specified"))
	UE_API void BP_OnDefaultOptionSpecified(int32 DefaultOptionIndex);
};

#undef UE_API
