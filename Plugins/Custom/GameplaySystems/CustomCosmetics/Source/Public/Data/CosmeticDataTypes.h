// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "CosmeticDataTypes.generated.h"

// 外观部件、复制列表和标签选择规则。

#define UE_API CUSTOMCOSMETICS_API

class UAnimInstance;
class UCosmeticsClientComponent;
struct FCharacterPartList;

/** 外观部件的碰撞策略。 */
UENUM(MinimalAPI)
enum class ECharacterPartCollisionMode : uint8
{
	/** 禁用生成部件的 Actor 碰撞。 */
	NoCollision,

	/** 保留部件类自身的碰撞设置。 */
	UseCollisionFromCharacterPart
};

/** 单个外观部件的类型、挂载点和碰撞配置。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FCharacterPart
{
	GENERATED_BODY()

	/** 外观 Actor 类型，仅在非专用服务器上生成。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PartClass;

	/** 外观部件的挂载插槽名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName;

	/** 生成外观 Actor 时使用的碰撞策略。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterPartCollisionMode CollisionMode = ECharacterPartCollisionMode::NoCollision;

	/** 比较部件类与插槽名称，不比较碰撞模式。 */
	bool operator==(const FCharacterPart& Other) const
	{
		return (PartClass == Other.PartClass) && (SocketName == Other.SocketName);
	}
};

/** 添加部件时分配的递增标识，用于定位并移除条目；不表示数组下标。 */
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

/** 调试外观覆盖常规部件的方式。 */
UENUM(MinimalAPI)
enum class ECosmeticCheatMode
{
	/** 隐藏常规部件并使用调试部件。 */
	ReplaceParts,
	/** 保留常规部件并追加调试部件。 */
	AddParts
};

/** 快速复制数组中的外观部件条目。 */
USTRUCT(MinimalAPI)
struct FAppliedCharacterPartEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FAppliedCharacterPartEntry() { ; }

	FString GetDebugString() const;

private:
	friend FCharacterPartList;
	friend UCosmeticsClientComponent;

	/** 该条目对应的外观部件描述。 */
	UPROPERTY()
	FCharacterPart Part;

	/** 仅服务端使用的条目标识，不参与复制。 */
	UPROPERTY(NotReplicated)
	int32 PartHandle = INDEX_NONE;

	/** 本机生成的 ChildActorComponent；专用服务器不创建该组件。 */
	UPROPERTY(NotReplicated)
	TObjectPtr<UChildActorComponent> SpawnedChildComponent = nullptr;
};

/** 复制外观描述，并管理对应本机外观 Actor 的快速数组。 */
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
	/** 通过快速数组复制的外观条目。 */
	UPROPERTY()
	TArray<FAppliedCharacterPartEntry> Entries;

	/** 持有该列表的外观组件，不参与复制。 */
	UPROPERTY(NotReplicated)
	TObjectPtr<UCosmeticsClientComponent> OwnerComponent;

	/** 为新增条目生成标识的递增计数器。 */
	int32 PartHandleCounter = 0;
};

template <>
struct TStructOpsTypeTraits<FCharacterPartList> : public TStructOpsTypeTraitsBase2<FCharacterPartList>
{
	enum { WithNetDeltaSerializer = true };
};

/** 外观部件的来源及常规部件的抑制状态。 */
enum class ECharacterPartSource : uint8
{
	/** 常规外观部件。 */
	Natural,

	/** 因调试覆盖而暂时停用的常规部件。 */
	NaturalSuppressedViaCheat,

	/** 通过开发者设置添加的调试部件。 */
	AppliedViaDeveloperSettingsCheat,

	/** 通过 CheatManager 命令添加的调试部件。 */
	AppliedViaCheatManager
};

/** 控制器保存的外观描述及其在当前 Pawn 上的应用状态。 */
USTRUCT(MinimalAPI, MinimalAPI)
struct FCharacterPartControllerEntry
{
	GENERATED_BODY()

	FCharacterPartControllerEntry() { ; }

public:
	/** 该记录对应的外观部件描述。 */
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FCharacterPart Part;

	/** 已应用到当前 Pawn 的部件句柄；未应用时无效。 */
	FCharacterPartHandle Handle;

	/** 部件来源及常规部件是否被调试覆盖。 */
	ECharacterPartSource Source = ECharacterPartSource::Natural;
};

/** 根据外观标签选择动画实例类的配置。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimLayerSelectionEntry
{
	GENERATED_BODY()

	/** 满足所需标签时选用的动画实例类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> Layer;

	/** 必须全部满足的外观标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Cosmetic"))
	FGameplayTagContainer RequiredTags;
};

/** 根据外观标签选择动画实例类的配置。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimLayerSelectionSet
{
	GENERATED_BODY()

	/** 按顺序匹配，使用首个标签满足且 Layer 非空的规则。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty=Layer))
	TArray<FCosmeticAnimLayerSelectionEntry> LayerRules;

	/** 没有匹配规则时使用的默认动画实例类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> DefaultLayer;

	/** 返回首个匹配规则的动画实例类，否则返回默认值。 */
	TSubclassOf<UAnimInstance> SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const;
};

/** 根据外观标签选择骨骼网格的配置。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimBodyStyleSelectionEntry
{
	GENERATED_BODY()

	/** 满足所需标签时选用的骨骼网格。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	/** 必须全部满足的外观标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Cosmetic"))
	FGameplayTagContainer RequiredTags;
};

/** 根据外观标签选择骨骼网格的配置。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FCosmeticAnimBodyStyleSelectionSet
{
	GENERATED_BODY()

	/** 按顺序匹配，使用首个标签满足且 Mesh 非空的规则。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty=Mesh))
	TArray<FCosmeticAnimBodyStyleSelectionEntry> MeshRules;

	/** 没有匹配 MeshRules 时使用的默认网格。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> DefaultMesh = nullptr;

	/** 更新外观网格时，优先使用的可选物理资产。 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPhysicsAsset> ForcedPhysicsAsset = nullptr;

	/** 返回首个匹配规则的骨骼网格，否则返回默认值。 */
	USkeletalMesh* SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const;
};


#undef UE_API
