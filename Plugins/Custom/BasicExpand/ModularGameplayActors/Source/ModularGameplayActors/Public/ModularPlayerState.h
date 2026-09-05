// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"

#include "ModularPlayerState.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

namespace EEndPlayReason
{
	enum Type : int;
}

class UObject;

/** 支持游戏扩展功能插件的最小类 */
UCLASS(MinimalAPI, Blueprintable)
class AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	//~ End AActor interface

protected:
	//~ Begin APlayerState interface
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	//~ End APlayerState interface
};

#undef UE_API
