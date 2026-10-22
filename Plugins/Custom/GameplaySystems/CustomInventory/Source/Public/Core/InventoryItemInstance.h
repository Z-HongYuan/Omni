// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameplayTagStack.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "InventoryItemInstance.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemFragmentBase;
class UInventoryItemDefinition;

/** 库存物品的运行时实例，保存定义类和可复制的属性标签计数。 */
UCLASS(BlueprintType, MinimalAPI)
class UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 注册实例需要复制的属性。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	/** 由权威端增加指定属性标签的计数。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 由权威端减少指定属性标签的计数。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 返回指定标签的计数；不存在时返回 0。 */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/** 返回实例是否持有指定属性标签。 */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	UE_API const UInventoryItemFragmentBase* FindFragmentByClass(TSubclassOf<UInventoryItemFragmentBase> FragmentClass) const;

	TSubclassOf<UInventoryItemDefinition> GetItemDef() const { return ItemDef; }

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return static_cast<const ResultClass*>(FindFragmentByClass(ResultClass::StaticClass()));
	}

private:
	void SetItemDef(TSubclassOf<UInventoryItemDefinition> InDef);

	friend struct FInventoryList;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	/** 该实例对应的物品定义类。 */
	UPROPERTY(Replicated)
	TSubclassOf<UInventoryItemDefinition> ItemDef;
};

#undef UE_API
