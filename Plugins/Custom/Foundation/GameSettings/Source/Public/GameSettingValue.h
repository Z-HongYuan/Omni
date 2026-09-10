// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingValue.generated.h"

#define UE_API GAMESETTINGS_API

class UObject;

//--------------------------------------
// UGameSettingValue
//--------------------------------------

/**
 * 所有概念上属于"值"的设置的基类，这些值可以被更改，
 * 因此也可以被重置或恢复到其初始值。
 */
UCLASS(MinimalAPI, Abstract)
class UGameSettingValue : public UGameSetting
{
	GENERATED_BODY()

public:
	UE_API UGameSettingValue();

	/** 为设置存储一个初始值。这将在初始化时调用，但如果你"应用"了该设置，也应调用它。 */
	virtual void StoreInitial() PURE_VIRTUAL(,);

	/** 将属性重置为默认值。 */
	virtual void ResetToDefault() PURE_VIRTUAL(,);

	/** 将设置恢复到初始值，即你在进行任何调整之前打开设置时的值。 */
	virtual void RestoreToInitial() PURE_VIRTUAL(,);

protected:
	UE_API virtual void OnInitialized() override;
};

#undef UE_API
