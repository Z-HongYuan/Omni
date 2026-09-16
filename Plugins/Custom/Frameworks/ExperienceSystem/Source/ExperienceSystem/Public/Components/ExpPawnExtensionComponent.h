// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "ExpPawnExtensionComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExtAbilitySystemComponent;
class UExpPawnData;

/*
 * 接管了 PawnData
 * 接管了 ASC 的各种处理
 * 接管了 整个组件的 初始化状态机
 * 会根据 PawnData 来初始化 ASC
 */
UCLASS(MinimalAPI, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UExpPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UExpPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 初始化状态接口使用的函数
	static UE_API const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;

	// 获取Pawn扩展组件
	UFUNCTION(BlueprintPure, Category = "Pawn")
	static UE_API UExpPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UExpPawnExtensionComponent>() : nullptr); }

	// 获取Pawn数据
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 将会在 GameMode 中直接设置了 PawnData
	UE_API void SetPawnData(const UExpPawnData* InPawnData);

	// ASC 的获取函数,只是转发缓存指针
	UFUNCTION(BlueprintPure, Category = "Pawn")
	UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const { return AbilitySystemComponent; }

	// 注册委托到 ASC 事件中
	UE_API void CallOrRegister_AbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate Delegate);
	// 注册委托到 ASC 事件中
	UE_API void Register_AbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate Delegate);
	// 注册 ASC, 需要在双端内调用
	UE_API void InitializeAbilitySystem(UExtAbilitySystemComponent* InASC, AActor* InOwnerActor);
	// 注销 ASC
	UE_API void UninitializeAbilitySystem();

	// 处理控制器变化, 将会在Pawn中调用
	UE_API void HandleControllerChanged();
	// 处理玩家状态变化, 将会在Pawn中调用
	UE_API void HandlePlayerStateReplicated();
	// 设置玩家输入组件, 将会在Pawn中调用
	UE_API void SetupPlayerInputComponent();

private:
	// 当拥有这个组件的Pawn成为 ASC的AvatarActor时调用
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	// 当拥有这个组件的Pawn不再是 ASC的AvatarActor时调用
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	// 缓存的 ASC 指针
	UPROPERTY(Transient)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;

	UFUNCTION()
	void OnRep_PawnData();

	// Pawn 使用的数据, 在延迟Spawn时设置,或者在放置的实例上设置
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Pawn")
	TObjectPtr<const UExpPawnData> PawnData;
};
#undef UE_API
