// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "ExtTaggedActor.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 提供 GameplayTag 查询接口，并在编辑器中禁用原生 Actor Tags 的编辑。 */
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
	/** 此 Actor 对外提供的静态 GameplayTag。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	FGameplayTagContainer StaticGameplayTags;
};

#undef UE_API
