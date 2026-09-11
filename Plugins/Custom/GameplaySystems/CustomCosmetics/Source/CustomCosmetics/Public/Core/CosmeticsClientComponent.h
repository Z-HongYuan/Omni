// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "Data/CosmeticDataTypes.h"
#include "CosmeticsClientComponent.generated.h"

#define UE_API CUSTOMCOSMETICS_API

// 角色部件已更换/已生成
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnedCharacterPartsChanged, UCosmeticsClientComponent*, ComponentWithChangedParts);

/*
 * 在客户端上 控制 角色外观的组件
 * 尽量使用 Controller 端控制外观请求
 * 
 * @TODO 需不需要初始化流程,需要依赖 Pawn Extention 吗
 */
UCLASS(MinimalAPI, ClassGroup=(Cosmetics), meta=(BlueprintSpawnableComponent))
class UCosmeticsClientComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UE_API UCosmeticsClientComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void OnRegister() override;
	//~End of UActorComponent interface

	// 仅在 Server 中调用 向角色添加外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API FCharacterPartHandle AddCharacterPart(const FCharacterPart& NewPart);

	// 仅在 Server 中调用 通过句柄移除对应的外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveCharacterPart(FCharacterPartHandle Handle);

	// 仅在 Server 中调用 移除所有添加的外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveAllCharacterParts();

	// 获取 所有已生成的 外观列表
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	UE_API TArray<AActor*> GetCharacterPartActors() const;

	// 获取 拥有者 的 SkeletalMeshComponent (如果不是 ACharacter 则返回 nullptr)
	UE_API USkeletalMeshComponent* GetParentMeshComponent() const;

	// 获取 拥有者 的 SkeletalMeshComponent (使用 Tag 进行过滤)
	UE_API USkeletalMeshComponent* GetTaggedMeshComponent() const;

	// 获取 拥有者 的 场景组件 (ACharacter 返回 Mesh, AActor 返回根组件, 其他 返回 nullptr)
	UE_API USceneComponent* GetSceneComponentToAttachTo() const;

	// 从所有外观中返回游戏标签集合，可选择仅筛选以指定根开头的标签
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	UE_API FGameplayTagContainer GetCombinedTags(FGameplayTag RequiredPrefix) const;

	UE_API void BroadcastChanged();
	UE_API void MakeSomeChangeToMesh();

	// 当生成的外观列表发生变化时将调用的委托
	UPROPERTY(BlueprintAssignable, Category=Cosmetics, BlueprintCallable)
	FSpawnedCharacterPartsChanged OnCharacterPartsChanged;

private:
	// 是否需要更新 Mesh 以适配外观
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	bool bIsNeedUpdateMesh = false;

	// 查询 MeshComponent 的标签
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FName FilterMeshTag = NAME_None;

	// 角色外观列表 (已复制)
	UPROPERTY(Replicated, Transient)
	FCharacterPartList CharacterPartList;

	// 基于外观的标签 选择 用于播放动画的 Mesh 规则
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FCosmeticAnimBodyStyleSelectionSet BodyMeshes;

	// 基于外观的标签 选择 用在 Mesh 上的动画实例
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FCosmeticAnimLayerSelectionSet AnimLayers;
};

#undef UE_API
