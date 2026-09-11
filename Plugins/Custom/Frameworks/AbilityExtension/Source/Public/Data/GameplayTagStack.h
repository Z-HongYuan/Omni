// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagStack.generated.h"

#define UE_API ABILITYEXTENSION_API

struct FGameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

/**
 * 表示一个游戏标签的叠加（标签+计数）
 */
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


/**
 * 游戏标签堆栈容器
 */
USTRUCT(BlueprintType)
struct FGameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UE_API FGameplayTagStackContainer() { ; }

	// 给标签添加指定数量的堆栈（如果StackCount低于1则无效）
	UE_API void AddStack(FGameplayTag Tag, int32 StackCount);

	// 从标签中移除指定数量的堆栈（如果StackCount低于1则无效）
	UE_API void RemoveStack(FGameplayTag Tag, int32 StackCount);

	// 返回指定标签的栈计数（如果标签不存在则返回0）
	UE_API int32 GetStackCount(FGameplayTag Tag) const
	{
		return TagToCountMap.FindRef(Tag);
	}

	// 如果至少存在一个指定标签的栈，则返回为真
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
	// 游戏标签堆叠复制列表
	UPROPERTY()
	TArray<FGameplayTagStack> Stacks;

	// 查询标签栈缓存加速列表
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
