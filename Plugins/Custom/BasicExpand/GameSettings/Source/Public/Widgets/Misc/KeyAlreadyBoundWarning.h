// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingPressAnyKey.h"
#include "KeyAlreadyBoundWarning.generated.h"

#define UE_API GAMESETTINGS_API

class UTextBlock;

/**
 * UKeyAlreadyBoundWarning
 * 带有文本块的"按任意键"界面，用于在按键已被绑定时向用户发出警告
 */
UCLASS(MinimalAPI, Abstract)
class UKeyAlreadyBoundWarning : public UGameSettingPressAnyKey
{
	GENERATED_BODY()

public:
	UE_API void SetWarningText(const FText& InText);

	UE_API void SetCancelText(const FText& InText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> WarningText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> CancelText;
};

#undef UE_API
