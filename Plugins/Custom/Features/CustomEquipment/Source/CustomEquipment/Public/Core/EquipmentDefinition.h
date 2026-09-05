// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "EquipmentDefinition.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UCustomAbilitySet;
class UEquipmentInstance;

USTRUCT()
struct FEquipmentActorToSpawn
{
	GENERATED_BODY()

	FEquipmentActorToSpawn() { ; }

	// 要生成的 Actor
	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	// Actor 的挂载点
	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	// Actor 的挂载点变换
	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * 定义可用于装备到角色的装备定义,静态定义
 */
UCLASS(MinimalAPI, Blueprintable, Const, Abstract, BlueprintType)
class UEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UEquipmentDefinition();

	// 生成的实例
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UEquipmentInstance> InstanceType;

	// 装备后会获得游戏能力集
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UCustomAbilitySet>> AbilitySetsToGrant;

	// 当角色装备后会生成在角色身上的Actor
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FEquipmentActorToSpawn> ActorsToSpawn;
};

#undef UE_API
