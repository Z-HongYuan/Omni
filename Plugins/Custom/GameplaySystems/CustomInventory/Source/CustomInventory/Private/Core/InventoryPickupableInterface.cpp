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
	// 如果Actor实现了接口就直接返回
	TScriptInterface<IInventoryPickupableInterface> PickupableActor(Actor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// 如果Actor不可拾取，查询其组件是否具有可拾取接口
	TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UInventoryPickupableInterface::StaticClass()) : TArray<UActorComponent*>();
	if (PickupableComponents.Num() > 0)
	{
		// 如果用户需要更复杂的拾取区分，就需要在别处解决。目前只返回第一个结果
		return TScriptInterface<IInventoryPickupableInterface>(PickupableComponents[0]);
	}

	return TScriptInterface<IInventoryPickupableInterface>();
}

void UPickupableStatics::AddPickupToInventory(UInventoryManager* InventoryComponent, TScriptInterface<IInventoryPickupableInterface> Pickup)
{
	if (!InventoryComponent || !Pickup) return;

	// 获取接口提供的拾取物品
	const FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();

	// 分别添加物品
	for (const FPickupTemplate& Template : PickupInventory.Templates)
	{
		InventoryComponent->AddItemDefinition(Template.ItemDef, Template.StackCount);
	}
	for (const FPickupInstance& Instance : PickupInventory.Instances)
	{
		InventoryComponent->AddItemInstance(Instance.Item);
	}
}
