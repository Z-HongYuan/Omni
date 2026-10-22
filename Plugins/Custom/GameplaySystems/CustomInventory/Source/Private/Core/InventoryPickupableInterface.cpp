// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryPickupableInterface.h"

#include "Core/InventoryManager.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryPickupableInterface)

UPickupableStatics::UPickupableStatics()
{
}

TScriptInterface<IInventoryPickupableInterface> UPickupableStatics::GetFirstPickupableFromActor(AActor* Actor)
{
	// 优先使用 Actor 自身提供的拾取内容。
	TScriptInterface<IInventoryPickupableInterface> PickupableActor(Actor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// Actor 未实现接口时，再查询其组件。
	TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UInventoryPickupableInterface::StaticClass()) : TArray<UActorComponent*>();
	if (PickupableComponents.Num() > 0)
	{
		// 多个组件实现接口时只取首个；更复杂的筛选由调用方另行处理。
		return TScriptInterface<IInventoryPickupableInterface>(PickupableComponents[0]);
	}

	return TScriptInterface<IInventoryPickupableInterface>();
}

void UPickupableStatics::AddPickupToInventory(UInventoryManager* InventoryComponent, TScriptInterface<IInventoryPickupableInterface> Pickup)
{
	if (!InventoryComponent || !Pickup) return;

	// 拾取内容可同时包含物品定义和已有实例。
	const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();

	// 按定义创建条目后，再尝试添加已有实例。
	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
	}
	for (const FPickupInstance& Instance : PickupInventory.Instances)
	{
		InventoryComponent->AddItemInstance(Instance.Item);
	}
}
