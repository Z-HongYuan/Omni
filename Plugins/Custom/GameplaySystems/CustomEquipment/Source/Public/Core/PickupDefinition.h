// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "PickupDefinition.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UNiagaraSystem;
class UInventoryItemDefinition;

/** 配置拾取物对应的库存物品、展示资源和重生冷却。 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const)
class UPickupDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 拾取后添加到库存的物品定义类。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Equipment")
	TSubclassOf<UInventoryItemDefinition> InventoryItemDefinition;

	/** 拾取物的展示模型。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	TObjectPtr<UStaticMesh> DisplayMesh;

	/** 拾取物重新生成的冷却时间，单位为秒。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	int32 SpawnCoolDownSeconds;

	/** 拾取时播放的音效。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> PickedUpSound;

	/** 重新生成时播放的音效。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> RespawnedSound;

	/** 拾取时播放的粒子特效。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickedUpEffect;

	/** 重新生成时播放的粒子特效。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> RespawnedEffect;
};

UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const)
class UWeaponPickupDefinition : public UPickupDefinition
{
	GENERATED_BODY()

public:
	/** 展示模型相对武器生成器的位置偏移。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	FVector WeaponMeshOffset;

	/** 展示模型的缩放。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Mesh")
	FVector WeaponMeshScale = FVector(1.0f, 1.0f, 1.0f);
};

#undef UE_API
