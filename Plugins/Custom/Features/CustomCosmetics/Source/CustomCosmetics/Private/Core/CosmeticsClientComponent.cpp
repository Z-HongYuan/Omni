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
	// 不广播通知
	CharacterPartList.ClearAllEntries(false);

	Super::EndPlay(EndPlayReason);
}

void UCosmeticsClientComponent::OnRegister()
{
	Super::OnRegister();

	// 不是模版类的话, 就设置复制数组的 Owner
	if (!IsTemplate())
	{
		CharacterPartList.SetOwnerComponent(this);
	}
}

FCharacterPartHandle UCosmeticsClientComponent::AddCharacterPart(const FCharacterPart& NewPart)
{
	// 在 快速复制数组中,已经有对于添加后的操作,例如添加一个Actor
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

	// 让观察者知道，例如是否需要进行团队着色或类似的处理
	OnCharacterPartsChanged.Broadcast(this);
}

void UCosmeticsClientComponent::MakeSomeChangeToMesh()
{
	// 不更改就不做任何事情
	if (!bIsNeedUpdateMesh) return;

	constexpr bool bReinitPose = true;

	// // 检查一个外观是否变化,来使用外观部件对 Mesh 的修改
	if (USkeletalMeshComponent* MeshComponent = GetTaggedMeshComponent())
	{
		// 根据外观部件标签确定要使用的网格
		const FGameplayTagContainer MergedTags = GetCombinedTags(FGameplayTag());
		USkeletalMesh* DesiredMesh = BodyMeshes.SelectBestBodyStyle(MergedTags);
		TSubclassOf<UAnimInstance> DesiredMeshAnimLayer = AnimLayers.SelectBestLayer(MergedTags);

		// 应用所需的网格（如果网格没有变化，这个调用是不可操作的）
		MeshComponent->SetSkeletalMesh(DesiredMesh, bReinitPose);
		MeshComponent->SetAnimInstanceClass(DesiredMeshAnimLayer);

		// 如果有强制覆盖，就应用所需的物理资产，而不是网格上的覆盖
		if (UPhysicsAsset* PhysicsAsset = BodyMeshes.ForcedPhysicsAsset)
		{
			MeshComponent->SetPhysicsAsset(PhysicsAsset, bReinitPose);
		}
	}
	// 如果这个没有任何东西的话,那么应该Mesh就不会改变,只会添加或者移除附加的Actor
}
