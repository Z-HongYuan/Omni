// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "Data/CosmeticDataTypes.h"
#include "CosmeticsClientComponent.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/** 外观部件生成、移除或替换后触发的委托。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnedCharacterPartsChanged, UCosmeticsClientComponent*, ComponentWithChangedParts);

/**
 * 复制 Pawn 的外观部件描述，并在非专用服务器上管理外观 Actor。
 * 外观请求通常由控制器组件维护，以便在 Pawn 更换后重新应用。
 * TODO: 确认是否需要接入 Pawn 扩展组件的初始化流程。
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

	/** 仅由服务端添加外观部件，并返回用于移除的句柄。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API FCharacterPartHandle AddCharacterPart(const FCharacterPart& NewPart);

	/** 仅由服务端按句柄移除对应外观部件。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveCharacterPart(FCharacterPartHandle Handle);

	/** 仅由服务端移除全部外观部件。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveAllCharacterParts();

	/** 返回本机已生成的外观 Actor；专用服务器不生成这些 Actor。 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	UE_API TArray<AActor*> GetCharacterPartActors() const;

	/** 返回所属 Character 的主网格组件；Owner 不是 Character 时返回 nullptr。 */
	UE_API USkeletalMeshComponent* GetParentMeshComponent() const;

	/** 在所属 Character 上按 FilterMeshTag 查找骨骼网格组件。 */
	UE_API USkeletalMeshComponent* GetTaggedMeshComponent() const;

	/** 优先返回 Character 的主网格，否则返回 Owner 的根组件。 */
	UE_API USceneComponent* GetSceneComponentToAttachTo() const;

	/** 收集已生成外观 Actor 的标签，并按需使用 RequiredPrefix 筛选。 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, BlueprintCosmetic, Category=Cosmetics)
	UE_API FGameplayTagContainer GetCombinedTags(FGameplayTag RequiredPrefix) const;

	UE_API void BroadcastChanged();
	UE_API void MakeSomeChangeToMesh();

	/** 已生成外观部件变化且网格更新完成后广播。 */
	UPROPERTY(BlueprintAssignable, Category=Cosmetics, BlueprintCallable)
	FSpawnedCharacterPartsChanged OnCharacterPartsChanged;

private:
	/** 是否按外观标签更新目标网格及动画实例。 */
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	bool bIsNeedUpdateMesh = false;

	/** 用于查找待更新骨骼网格组件的组件标签。 */
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FName FilterMeshTag = NAME_None;

	/** 可复制的外观部件列表，负责本机外观 Actor 的生命周期。 */
	UPROPERTY(Replicated, Transient)
	FCharacterPartList CharacterPartList;

	/** 根据外观标签选择骨骼网格的规则。 */
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FCosmeticAnimBodyStyleSelectionSet BodyMeshes;

	/** 根据外观标签选择目标网格动画实例类的规则。 */
	UPROPERTY(EditAnywhere, Category=Cosmetics, meta = (EditCondition = "bIsNeedUpdateMesh"))
	FCosmeticAnimLayerSelectionSet AnimLayers;
};

#undef UE_API
