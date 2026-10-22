// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "ExpPawnExtensionComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExtAbilitySystemComponent;
class UExpPawnData;

/**
 * 保存并复制 PawnData，协调 Pawn 各功能的数据就绪屏障。
 * 由调用方提供 ASC 和 Owner，负责绑定 Avatar、应用标签关系映射及注销通知，不创建 ASC。
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

	// 初始化状态接口。
	static UE_API const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;

	/** 查找 Actor 上的 Pawn 扩展组件；Actor 为空时返回空指针。 */
	UFUNCTION(BlueprintPure, Category = "Pawn")
	static UE_API UExpPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UExpPawnExtensionComponent>() : nullptr); }

	/** 按指定类型读取 PawnData，类型不符或尚未设置时返回空指针。 */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** 由权威端设置 PawnData 并推进初始化；已有数据时拒绝覆盖。 */
	UE_API void SetPawnData(const UExpPawnData* InPawnData);

	/** 返回当前缓存的 ASC，不创建或初始化能力系统。 */
	UFUNCTION(BlueprintPure, Category = "Pawn")
	UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** 按委托绑定对象去重注册；ASC 已初始化时还会立即执行本次委托。 */
	UE_API void CallOrRegister_AbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate Delegate);
	/** 按委托绑定对象去重注册 ASC 注销通知。 */
	UE_API void Register_AbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate Delegate);
	/** 绑定外部 ASC 与 Owner，服务器和客户端分别调用；若已有旧 Avatar，则先解除旧绑定。 */
	UE_API void InitializeAbilitySystem(UExtAbilitySystemComponent* InASC, AActor* InOwnerActor);
	/** 解除缓存；仅当当前 Pawn 仍为 Avatar 时取消非豁免能力、清理输入和 Cue，并广播注销。 */
	UE_API void UninitializeAbilitySystem();

	/** 由 Pawn 在控制器变化时调用，刷新当前 Avatar 的 ActorInfo 并推进初始化。 */
	UE_API void HandleControllerChanged();
	/** 由 Pawn 在 PlayerState 复制完成后调用，重新检查初始化条件。 */
	UE_API void HandlePlayerStateReplicated();
	/** 由 Pawn 在输入组件设置后调用，重新检查初始化条件，不在此绑定输入。 */
	UE_API void SetupPlayerInputComponent();

private:
	/** 本 Pawn 完成 ASC Avatar 绑定后的通知。 */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	/** 本组件解除当前 Pawn 的 ASC Avatar 绑定时广播的通知。 */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	/** 外部持有的 ASC 缓存；本组件管理 Avatar 绑定，不拥有其创建生命周期。 */
	UPROPERTY(Transient)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;

	UFUNCTION()
	void OnRep_PawnData();

	/** Pawn 的复制配置，可在延迟生成时由服务器设置，或在关卡实例上配置。 */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Pawn")
	TObjectPtr<const UExpPawnData> PawnData;
};
#undef UE_API
