// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Data/GameplayTagStack.h"
#include "CustomTagsStackComponent.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

/*
 * 提供FGameplayTagStackContainer的组件
 * 提供TagStack使用
 * 标签的堆栈容器
 */
UCLASS(MinimalAPI, meta=(BlueprintSpawnableComponent))
class UCustomTagsStackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomTagsStackComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=TagsStack)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 返回指定标签的栈计数（如果标签不存在则返回0）
	UFUNCTION(BlueprintCallable, Category=TagsStack)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// 从标签中移除指定数量的堆栈（如果StackCount低于1则无效）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=TagsStack)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 如果至少存在一个指定标签的栈，则返回为真
	UFUNCTION(BlueprintCallable, Category=TagsStack)
	bool HasStatTag(FGameplayTag Tag) const;

protected:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer GameplayTagStackContainer;
};
#undef UE_API
