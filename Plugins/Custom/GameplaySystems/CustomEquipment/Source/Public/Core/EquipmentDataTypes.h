// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/ExtAbilitySet.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "EquipmentDataTypes.generated.h"

class UEquipmentInstance;
class UEquipmentDefinition;
class UEquipmentManager;
struct FEquipmentList;

/** 快速数组中的单个装备条目，记录定义、实例及已授予能力。 */
USTRUCT(BlueprintType)
struct FAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAppliedEquipmentEntry() { ; }

	FString GetDebugString() const;

private:
	friend FEquipmentList;
	friend UEquipmentManager;

	/** 创建该装备实例所用的定义类。 */
	UPROPERTY()
	TSubclassOf<UEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UEquipmentInstance> Instance = nullptr;

	/** 仅由权威端保存的能力授予句柄，用于卸装时撤销能力。 */
	UPROPERTY(NotReplicated)
	FExtAbilitySet_GrantedHandles GrantedHandles;
};


/** 复制装备条目，并在增删时管理能力与外观 Actor 的生命周期。 */
USTRUCT(BlueprintType)
struct FEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FEquipmentList() : OwnerComponent(nullptr) { ; }

	FEquipmentList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) { ; }

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAppliedEquipmentEntry, FEquipmentList>(Entries, DeltaParms, *this);
	}

	/** 由权威端创建装备实例，授予能力并生成外观 Actor。 */
	UEquipmentInstance* AddEntry(TSubclassOf<UEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(UEquipmentInstance* Instance);

private:
	UExtAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UEquipmentManager;

	/** 通过快速数组复制的装备条目。 */
	UPROPERTY()
	TArray<FAppliedEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FEquipmentList> : public TStructOpsTypeTraitsBase2<FEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};
