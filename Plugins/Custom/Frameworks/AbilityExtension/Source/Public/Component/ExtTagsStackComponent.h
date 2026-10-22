// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Data/GameplayTagStack.h"
#include "GameFramework/Actor.h"
#include "ExtTagsStackComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 以组件形式提供可复制的标签计数容器。 */
UCLASS(MinimalAPI, meta=(BlueprintSpawnableComponent))
class UExtTagsStackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExtTagsStackComponent();

	/** 查找 Actor 上的标签堆栈组件；Actor 为空或未挂载时返回空指针。 */
	UFUNCTION(BlueprintPure, Category=TagsStack, meta=(DefaultToSelf="Actor"))
	static UE_API UExtTagsStackComponent* FindTagsStack(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UExtTagsStackComponent>() : nullptr; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=TagsStack)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 返回指定标签的计数，不存在时返回零。 */
	UFUNCTION(BlueprintCallable, Category=TagsStack)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/** 移除指定数量的标签计数，StackCount 小于一时不操作。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=TagsStack)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** 检查是否存在指定标签的计数记录。 */
	UFUNCTION(BlueprintCallable, Category=TagsStack)
	bool HasStatTag(FGameplayTag Tag) const;

protected:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer GameplayTagStackContainer;
};
#undef UE_API
