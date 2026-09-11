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

/** 模块化玩家状态，支持组件扩展，并向玩家状态组件转发重置与属性复制。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	//~AActor 接口结束

protected:
	//~APlayerState 接口
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	//~APlayerState 接口结束
};

#undef UE_API
