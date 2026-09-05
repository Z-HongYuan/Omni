// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameplayTagStack.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "InventoryItemInstance.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemFragmentBase;
class UInventoryItemDefinition;

/**
 * 运行时的库存实例,支持堆叠, 例如 木材*100
 */
UCLASS(BlueprintType, MinimalAPI)
class UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 注册所有复制片段 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	// 向实例添加堆叠标签计数
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 向实例移除堆叠标签计数
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 返回指定标签的堆叠计数（不存在则返回0）
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// 返回指定标签是否存在堆叠
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

	// 实例的CDO定义
	UPROPERTY(Replicated)
	TSubclassOf<UInventoryItemDefinition> ItemDef;
};

#undef UE_API
