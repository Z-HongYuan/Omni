// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/CosmeticsClientComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CosmeticsClientComponent)

UCosmeticsClientComponent::UCosmeticsClientComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCosmeticsClientComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CharacterPartList);
}

void UCosmeticsClientComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCosmeticsClientComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 结束生命周期时清理部件，避免再向外广播变化。
	CharacterPartList.ClearAllEntries(false);

	Super::EndPlay(EndPlayReason);
}

void UCosmeticsClientComponent::OnRegister()
{
	Super::OnRegister();

	// 仅为运行时组件实例关联复制列表的 Owner。
	if (!IsTemplate())
	{
		CharacterPartList.SetOwnerComponent(this);
	}
}

FCharacterPartHandle UCosmeticsClientComponent::AddCharacterPart(const FCharacterPart& NewPart)
{
	// 列表负责生成部件并通知外观变化。
	return CharacterPartList.AddEntry(NewPart);
}

void UCosmeticsClientComponent::RemoveCharacterPart(FCharacterPartHandle Handle)
{
	CharacterPartList.RemoveEntry(Handle);
}

void UCosmeticsClientComponent::RemoveAllCharacterParts()
{
	CharacterPartList.ClearAllEntries(true);
}

TArray<AActor*> UCosmeticsClientComponent::GetCharacterPartActors() const
{
	TArray<AActor*> Result;
	Result.Reserve(CharacterPartList.Entries.Num());

	for (const FAppliedCharacterPartEntry& Entry : CharacterPartList.Entries)
	{
		if (UChildActorComponent* PartComponent = Entry.SpawnedChildComponent)
		{
			if (AActor* SpawnedActor = PartComponent->GetChildActor())
			{
				Result.Add(SpawnedActor);
			}
		}
	}

	return Result;
}

USkeletalMeshComponent* UCosmeticsClientComponent::GetParentMeshComponent() const
{
	if (AActor* OwnerActor = GetOwner())
		if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
			if (USkeletalMeshComponent* MeshComponent = OwningCharacter->GetMesh())
				return MeshComponent;

	return nullptr;
}

USkeletalMeshComponent* UCosmeticsClientComponent::GetTaggedMeshComponent() const
{
	if (AActor* OwnerActor = GetOwner())
		if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
			if (USkeletalMeshComponent* MeshComponent = OwningCharacter->FindComponentByTag<USkeletalMeshComponent>(FilterMeshTag))
				return MeshComponent;

	return nullptr;
}

USceneComponent* UCosmeticsClientComponent::GetSceneComponentToAttachTo() const
{
	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		return MeshComponent;
	}
	if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetRootComponent();
	}

	return nullptr;
}

FGameplayTagContainer UCosmeticsClientComponent::GetCombinedTags(FGameplayTag RequiredPrefix) const
{
	FGameplayTagContainer Result = CharacterPartList.CollectCombinedTags();
	if (RequiredPrefix.IsValid())
	{
		return Result.Filter(FGameplayTagContainer(RequiredPrefix));
	}

	return Result;
}

void UCosmeticsClientComponent::BroadcastChanged()
{
	MakeSomeChangeToMesh();

	// 网格更新后再通知观察者，便于重新应用队伍颜色等外观。
	OnCharacterPartsChanged.Broadcast(this);
}

void UCosmeticsClientComponent::MakeSomeChangeToMesh()
{
	// 只有启用网格联动时才应用外观标签规则。
	if (!bIsNeedUpdateMesh) return;

	constexpr bool bReinitPose = true;

	// 找到配置标签对应的网格后，再应用部件驱动的外观。
	if (USkeletalMeshComponent* MeshComponent = GetTaggedMeshComponent())
	{
		// 按部件标签分别选择网格和动画实例类。
		const FGameplayTagContainer MergedTags = GetCombinedTags(FGameplayTag());
		USkeletalMesh* DesiredMesh = BodyMeshes.SelectBestBodyStyle(MergedTags);
		TSubclassOf<UAnimInstance> DesiredMeshAnimLayer = AnimLayers.SelectBestLayer(MergedTags);

		// 将规则选出的网格与动画实例类应用到目标组件。
		MeshComponent->SetSkeletalMesh(DesiredMesh, bReinitPose);
		MeshComponent->SetAnimInstanceClass(DesiredMeshAnimLayer);

		// 配置了强制物理资产时，覆盖网格自身的物理资产。
		if (UPhysicsAsset* PhysicsAsset = BodyMeshes.ForcedPhysicsAsset)
		{
			MeshComponent->SetPhysicsAsset(PhysicsAsset, bReinitPose);
		}
	}
	// 未找到目标网格时，部件增删仍有效，但不会更换网格。
}
