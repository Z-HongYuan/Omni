// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "EquipmentDefinition.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UExtAbilitySet;
class UEquipmentInstance;

USTRUCT()
struct FEquipmentActorToSpawn
{
	GENERATED_BODY()

	FEquipmentActorToSpawn() { ; }

	/** 装备时生成的外观 Actor 类型。 */
	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	/** 外观 Actor 使用的挂载点名称。 */
	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	/** 外观 Actor 相对挂载点的变换。 */
	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/** 装备的静态配置，指定实例类型、能力集和外观 Actor。 */
UCLASS(MinimalAPI, Blueprintable, Const, Abstract, BlueprintType)
class UEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UEquipmentDefinition();

	/** 创建装备时使用的实例类型。 */
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UEquipmentInstance> InstanceType;

	/** 权威端在装备时授予、卸装时撤销的能力集。 */
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UExtAbilitySet>> AbilitySetsToGrant;

	/** 装备时生成并挂接到 Pawn 的外观 Actor 配置。 */
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FEquipmentActorToSpawn> ActorsToSpawn;
};

#undef UE_API
