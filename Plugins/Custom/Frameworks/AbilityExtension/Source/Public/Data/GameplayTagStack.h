// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagStack.generated.h"

#define UE_API ABILITYEXTENSION_API

struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

/** 可复制的游戏标签及其计数。 */
USTRUCT(BlueprintType)
struct FGameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UE_API FGameplayTagStack() { ; }

	UE_API FGameplayTagStack(FGameplayTag InTag, int32 InStackCount) : Tag(InTag), StackCount(InStackCount) { ; }

	UE_API FString GetDebugString() const
	{
		return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
	};

private:
	friend FGameplayTagStackContainer;

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 StackCount = 0;
};


/** 通过 Fast Array 复制标签计数，并维护本地查询缓存。 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UE_API FGameplayTagStackContainer() { ; }

	/** 增加指定标签的计数；无效标签或 StackCount 小于一时不操作。 */
	UE_API void AddStack(FGameplayTag Tag, int32 StackCount);

	/** 减少指定标签的计数，减至零时删除记录；StackCount 小于一时不操作。 */
	UE_API void RemoveStack(FGameplayTag Tag, int32 StackCount);

	/** 返回指定标签的计数，不存在时返回零。 */
	UE_API int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	/** 检查是否存在指定标签的计数记录。 */
	UE_API bool ContainsTag(FGameplayTag Tag) const
	{
		return TagToCountMap.Contains(Tag);
	}

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	UE_API bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameplayTagStack, FGameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}

private:
	/** 用于 Fast Array 增量复制的标签计数列表。 */
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;

	/** 标签计数查询缓存，由本地修改及复制回调同步更新。 */
	TMap<FGameplayTag, int32> TagToCountMap;
};

template <>
struct TStructOpsTypeTraits<FGameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

#undef UE_API
