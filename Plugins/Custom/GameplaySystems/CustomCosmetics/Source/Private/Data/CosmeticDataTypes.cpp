// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/CosmeticDataTypes.h"
#include "Core/CosmeticsClientComponent.h"
#include "GameplayTagAssetInterface.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CosmeticDataTypes)

FString FAppliedCharacterPartEntry::GetDebugString() const
{
	return FString::Printf(TEXT("(PartClass: %s, Socket: %s, Instance: %s)"), *GetPathNameSafe(Part.PartClass), *Part.SocketName.ToString(), *GetPathNameSafe(SpawnedChildComponent));
}

void FCharacterPartList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// 复制移除条目前先销毁本机部件，实际发生销毁时再通知外观变化。
	bool bDestroyedAnyActors = false;
	for (int32 Index : RemovedIndices)
	{
		FAppliedCharacterPartEntry& Entry = Entries[Index];
		bDestroyedAnyActors |= DestroyActorForEntry(Entry);
	}

	if (bDestroyedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

void FCharacterPartList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// 为复制新增条目生成本机部件，实际生成成功时再通知外观变化。
	bool bCreatedAnyActors = false;
	for (int32 Index : AddedIndices)
	{
		FAppliedCharacterPartEntry& Entry = Entries[Index];
		bCreatedAnyActors |= SpawnActorForEntry(Entry);
	}

	if (bCreatedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

void FCharacterPartList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	bool bChangedAnyActors = false;

	// 描述变化时重建对应部件，统一处理类型、挂载点和碰撞配置。
	for (int32 Index : ChangedIndices)
	{
		FAppliedCharacterPartEntry& Entry = Entries[Index];

		// 累积本轮是否发生销毁或生成，循环后只广播一次。
		bChangedAnyActors |= DestroyActorForEntry(Entry);
		bChangedAnyActors |= SpawnActorForEntry(Entry);
	}

	if (bChangedAnyActors && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

FCharacterPartHandle FCharacterPartList::AddEntry(FCharacterPart NewPart)
{
	FCharacterPartHandle Result;
	Result.PartHandle = PartHandleCounter++;

	if (ensure(OwnerComponent && OwnerComponent->GetOwner() && OwnerComponent->GetOwner()->HasAuthority()))
	{
		FAppliedCharacterPartEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Part = NewPart;
		NewEntry.PartHandle = Result.PartHandle;

		if (SpawnActorForEntry(NewEntry))
		{
			OwnerComponent->BroadcastChanged();
		}

		MarkItemDirty(NewEntry);
	}

	return Result;
}

void FCharacterPartList::RemoveEntry(FCharacterPartHandle Handle)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FAppliedCharacterPartEntry& Entry = *EntryIt;
		if (Entry.PartHandle == Handle.PartHandle)
		{
			const bool bDestroyedActor = DestroyActorForEntry(Entry);
			EntryIt.RemoveCurrent();
			MarkArrayDirty();

			if (bDestroyedActor && ensure(OwnerComponent))
			{
				OwnerComponent->BroadcastChanged();
			}

			break;
		}
	}
}

void FCharacterPartList::ClearAllEntries(bool bBroadcastChangeDelegate)
{
	bool bDestroyedAnyActors = false;
	for (FAppliedCharacterPartEntry& Entry : Entries)
	{
		bDestroyedAnyActors |= DestroyActorForEntry(Entry);
	}
	Entries.Reset();
	MarkArrayDirty();

	if (bDestroyedAnyActors && bBroadcastChangeDelegate && ensure(OwnerComponent))
	{
		OwnerComponent->BroadcastChanged();
	}
}

FGameplayTagContainer FCharacterPartList::CollectCombinedTags() const
{
	FGameplayTagContainer Result;

	for (const FAppliedCharacterPartEntry& Entry : Entries)
	{
		if (Entry.SpawnedChildComponent != nullptr)
		{
			if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Entry.SpawnedChildComponent->GetChildActor()))
			{
				TagInterface->GetOwnedGameplayTags(Result);
			}
		}
	}

	return Result;
}

bool FCharacterPartList::SpawnActorForEntry(FAppliedCharacterPartEntry& Entry)
{
	bool bCreatedAnyActors = false;

	// 专用服务器只复制描述；客户端、监听服务器和单机负责生成表现。
	if (ensure(OwnerComponent) && !OwnerComponent->IsNetMode(NM_DedicatedServer))
	{
		if (Entry.Part.PartClass != nullptr)
		{
			// UWorld* World = OwnerComponent->GetWorld();

			if (USceneComponent* ComponentToAttachTo = OwnerComponent->GetSceneComponentToAttachTo())
			{
				// const FTransform SpawnTransform = ComponentToAttachTo->GetSocketTransform(Entry.Part.SocketName);

				UChildActorComponent* PartComponent = NewObject<UChildActorComponent>(OwnerComponent->GetOwner());

				PartComponent->SetupAttachment(ComponentToAttachTo, Entry.Part.SocketName);
				PartComponent->SetChildActorClass(Entry.Part.PartClass);
				PartComponent->RegisterComponent();

				if (AActor* SpawnedActor = PartComponent->GetChildActor())
				{
					switch (Entry.Part.CollisionMode)
					{
					case ECharacterPartCollisionMode::UseCollisionFromCharacterPart:
						// 保留部件类自身的碰撞设置。
						break;

					case ECharacterPartCollisionMode::NoCollision:
						SpawnedActor->SetActorEnableCollision(false);
						break;
					}

					// 让部件根组件在挂载目标更新后 Tick，保持挂接更新顺序。
					if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
					{
						SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
					}
				}

				Entry.SpawnedChildComponent = PartComponent;
				bCreatedAnyActors = true;
			}
		}
	}

	return bCreatedAnyActors;
}

bool FCharacterPartList::DestroyActorForEntry(FAppliedCharacterPartEntry& Entry)
{
	bool bDestroyedAnyActors = false;

	if (Entry.SpawnedChildComponent != nullptr)
	{
		Entry.SpawnedChildComponent->DestroyComponent();
		Entry.SpawnedChildComponent = nullptr;
		bDestroyedAnyActors = true;
	}

	return bDestroyedAnyActors;
}

TSubclassOf<UAnimInstance> FCosmeticAnimLayerSelectionSet::SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const
{
	for (const FCosmeticAnimLayerSelectionEntry& Rule : LayerRules)
	{
		if ((Rule.Layer != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Layer;
		}
	}

	return DefaultLayer;
}

USkeletalMesh* FCosmeticAnimBodyStyleSelectionSet::SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const
{
	for (const FCosmeticAnimBodyStyleSelectionEntry& Rule : MeshRules)
	{
		if ((Rule.Mesh != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Mesh;
		}
	}

	return DefaultMesh;
}
