// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/CustomAbilitySet.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "EquipmentDataTypes.generated.h"

class UEquipmentInstance;
class UEquipmentDefinition;
class UEquipmentManager;
struct FEquipmentList;

/** 单个装备项目 继承于快速复制的基类 */
USTRUCT(BlueprintType)
struct FAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAppliedEquipmentEntry() { ; }

	FString GetDebugString() const;

private:
	friend FEquipmentList;
	friend UEquipmentManager;

	// 被装备的CDO类别
	UPROPERTY()
	TSubclassOf<UEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UEquipmentInstance> Instance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FCustomAbilitySet_GrantedHandles GrantedHandles;
};


/** 装备的装备 */
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

	UEquipmentInstance* AddEntry(TSubclassOf<UEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(UEquipmentInstance* Instance);

private:
	UCustomAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UEquipmentManager;

	// 需要复制的装备数据
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
