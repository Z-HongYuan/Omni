// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "PickupDefinition.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UNiagaraSystem;
class UInventoryItemDefinition;

/**
 * 
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const)
class UPickupDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	//定义拾取物的演员生成对象、赋予的能力以及添加标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Equipment")
	TSubclassOf<UInventoryItemDefinition> InventoryItemDefinition;

	//拾取的视觉模型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	TObjectPtr<UStaticMesh> DisplayMesh;

	//拾取之间的冷却时间
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	int32 SpawnCoolDownSeconds;

	//拾取时的音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> PickedUpSound;

	//拾取重新生成时播放的声音
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> RespawnedSound;

	//拾起时播放的粒子特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickedUpEffect;

	//拾取物重生时播放的粒子效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> RespawnedEffect;
};

UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const)
class UWeaponPickupDefinition : public UPickupDefinition
{
	GENERATED_BODY()

public:
	//设置显示网格在武器生成器上方的高度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	FVector WeaponMeshOffset;

	//设置显示网格在武器生成器上方的高度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	FVector WeaponMeshScale = FVector(1.0f, 1.0f, 1.0f);
};

#undef UE_API
