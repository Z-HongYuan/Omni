// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "ExtTaggedActor.generated.h"

#define UE_API ABILITYEXTENSION_API

/*
 * 使用 GameplayTag 替代了 UE 原生 Actor 的标签
 */
UCLASS(MinimalAPI)
class AExtTaggedActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UE_API AExtTaggedActor();

	//~IGameplayTagAssetInterface
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~End of UObject interface

protected:
	// 与Actor相关的游戏性标签
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	FGameplayTagContainer StaticGameplayTags;
};

#undef UE_API
