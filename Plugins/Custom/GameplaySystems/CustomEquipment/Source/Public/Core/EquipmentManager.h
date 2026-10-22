// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "EquipmentDataTypes.h"
#include "Components/PawnComponent.h"
#include "EquipmentManager.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UEquipmentInstance;
class UEquipmentDefinition;

/** 管理 Pawn 的装备实例、能力授予及子对象复制。 */
UCLASS(MinimalAPI, MinimalAPI, BlueprintType, Const, meta=(BlueprintSpawnableComponent))
class UEquipmentManager : public UPawnComponent
{
	GENERATED_BODY()

public:
	UE_API UEquipmentManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UE_API UEquipmentInstance* EquipItem(TSubclassOf<UEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UE_API void UnequipItem(UEquipmentInstance* ItemInstance);

	//~UObject interface
	UE_API virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface

	//~UActorComponent interface
	//virtual void EndPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void InitializeComponent() override;
	UE_API virtual void UninitializeComponent() override;
	UE_API virtual void ReadyForReplication() override;
	//~End of UActorComponent interface

	/** 返回首个匹配类型的装备实例；没有匹配项时返回 nullptr。 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UE_API UEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UEquipmentInstance> InstanceType);

	/** 返回所有匹配类型的装备实例；没有匹配项时返回空数组。 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UE_API TArray<UEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UEquipmentInstance> InstanceType) const;

	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return static_cast<T*>(GetFirstInstanceOfType(T::StaticClass()));
	}

private:
	UPROPERTY(Replicated)
	FEquipmentList EquipmentList;
};

#undef UE_API
