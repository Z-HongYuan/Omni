// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "CosmeticDataTypes.generated.h"

/*
 * 声明换装/外观系统的所有使用的数据类型
 */

#define UE_API CUSTOMCOSMETICS_API

class UAnimInstance;
class UCosmeticsClientComponent;
struct FCharacterPartList;

// 角色部件使用的 碰撞模式
UENUM(MinimalAPI)
enum class ECharacterPartCollisionMode : uint8
{
	// 部件没有碰撞
	NoCollision,

	// 部件使用碰撞
	UseCollisionFromCharacterPart
};

// 单个角色部件
USTRUCT(BlueprintType, MinimalAPI)
struct FCharacterPart
{
	GENERATED_BODY()

	// 角色部件的表现类, 会在客户端上生成
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PartClass;

	// 需要挂载到的插槽名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName;

	// 此角色部件的碰撞模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterPartCollisionMode CollisionMode = ECharacterPartCollisionMode::NoCollision;

	// 比较两个角色部件是否相等
	bool operator==(const FCharacterPart& Other) const
	{
		return (PartClass == Other.PartClass) && (SocketName == Other.SocketName);
	}
};

// 当添加 角色部件后 返回的操作句柄,能够通过此句柄移除
// 记录数组中的索引
USTRUCT(BlueprintType, MinimalAPI)
struct FCharacterPartHandle
{
	GENERATED_BODY()

	void Reset()
	{
		PartHandle = INDEX_NONE;
	}

	bool IsValid() const
	{
		return PartHandle != INDEX_NONE;
	}

private:
	UPROPERTY()
	int32 PartHandle = INDEX_NONE;

	friend FCharacterPartList;
};

// 使用的作弊模式
UENUM(MinimalAPI)
enum class ECosmeticCheatMode
{
	// 更换角色部件
	ReplaceParts,
	// 添加角色部件
	AddParts
};

// 应用角色部件时的单个 Item
USTRUCT(MinimalAPI)
struct FAppliedCharacterPartEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAppliedCharacterPartEntry() { ; }

	FString GetDebugString() const;

private:
	friend FCharacterPartList;
	friend UCosmeticsClientComponent;

	// 这个数组项目中代表的角色部件
	UPROPERTY()
	FCharacterPart Part;

	// 仅服务器 中使用的控制句柄索引
	UPROPERTY(NotReplicated)
	int32 PartHandle = INDEX_NONE;

	// 仅客户端 生成的演员实例
	UPROPERTY(NotReplicated)
	TObjectPtr<UChildActorComponent> SpawnedChildComponent = nullptr;
};

// 快速复制的 外观部件列表/数组
USTRUCT(BlueprintType, MinimalAPI)
struct FCharacterPartList : public FFastArraySerializer
{
	GENERATED_BODY()

	FCharacterPartList() : OwnerComponent(nullptr) { ; }

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAppliedCharacterPartEntry, FCharacterPartList>(Entries, DeltaParms, *this);
	}

	FCharacterPartHandle AddEntry(FCharacterPart NewPart);
	void RemoveEntry(FCharacterPartHandle Handle);
	void ClearAllEntries(bool bBroadcastChangeDelegate);

	FGameplayTagContainer CollectCombinedTags() const;

	void SetOwnerComponent(UCosmeticsClientComponent* InOwnerComponent)
	{
		OwnerComponent = InOwnerComponent;
	}

private:
	friend UCosmeticsClientComponent;

	bool SpawnActorForEntry(FAppliedCharacterPartEntry& Entry);
	bool DestroyActorForEntry(FAppliedCharacterPartEntry& Entry);

private:
	// 数组的快速复制列表
	UPROPERTY()
	TArray<FAppliedCharacterPartEntry> Entries;

	// 包含此数组的 组件
	UPROPERTY(NotReplicated)
	TObjectPtr<UCosmeticsClientComponent> OwnerComponent;

	// 数组 Item 的句柄计数器
	int32 PartHandleCounter = 0;
};

template <>
struct TStructOpsTypeTraits<FCharacterPartList> : public TStructOpsTypeTraitsBase2<FCharacterPartList>
{
	enum { WithNetDeltaSerializer = true };
};

// 外观部件来源
enum class ECharacterPartSource : uint8
{
	// 原生
	Natural,

	// 原生, 但能被作弊覆盖
	NaturalSuppressedViaCheat,

	// 使用开发者设置作弊 添加/替换外观
	AppliedViaDeveloperSettingsCheat,

	// 使用作弊管理器 添加/替换外观
	AppliedViaCheatManager
};

// 控制器保存的 外观部件(应该与客户端保持一致)
USTRUCT(MinimalAPI, MinimalAPI)
struct FCharacterPartControllerEntry
{
	GENERATED_BODY()

	FCharacterPartControllerEntry() { ; }

public:
	// 代表的 外观部件
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FCharacterPart Part;

	// 外观部件的句柄 (如果已经应用了)
	FCharacterPartHandle Handle;

	// 外观部件的来源
	ECharacterPartSource Source = ECharacterPartSource::Natural;
};

// 动画层选择
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimLayerSelectionEntry
{
	GENERATED_BODY()

	//如果标签匹配，则应用动画层
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> Layer;

	// 需要外观标签（所有这些标签都必须存在才能被视为匹配）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Cosmetic"))
	FGameplayTagContainer RequiredTags;
};

// 动画层选择
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimLayerSelectionSet
{
	GENERATED_BODY()

	// 要应用的动画规则列表，将使用第一个匹配的动画规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty=Layer))
	TArray<FCosmeticAnimLayerSelectionEntry> LayerRules;

	// 如果所有 LayerRules 都不匹配，则使用的默认层
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> DefaultLayer;

	// 根据规则选择最佳图层
	TSubclassOf<UAnimInstance> SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const;
};

// Mesh 样式
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimBodyStyleSelectionEntry
{
	GENERATED_BODY()

	// 如果标签匹配，则应用动画层
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	// 需要外观标签（所有这些标签都必须存在才能被视为匹配）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Cosmetic"))
	FGameplayTagContainer RequiredTags;
};

// Mesh 样式
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimBodyStyleSelectionSet
{
	GENERATED_BODY()

	// 要应用的 Mesh 规则列表，将使用第一个匹配的 Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty=Mesh))
	TArray<FCosmeticAnimBodyStyleSelectionEntry> MeshRules;

	// 如果所有 LayerRules 都不匹配，则使用的默认Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> DefaultMesh = nullptr;

	// 成功设置后使用的物理资源
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPhysicsAsset> ForcedPhysicsAsset = nullptr;

	// 根据规则选择最佳的身体样式骨架网格
	USkeletalMesh* SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const;
};


#undef UE_API
