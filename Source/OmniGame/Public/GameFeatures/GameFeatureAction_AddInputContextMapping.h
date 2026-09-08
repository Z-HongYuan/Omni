// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"

#include "GameFeatureAction_AddInputContextMapping.generated.h"

#define UE_API OMNIGAME_API

class AActor;
class UInputMappingContext;
class UPlayer;
class APlayerController;
struct FComponentRequestHandle;

USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// 优先级更高的映射会覆盖优先级更低的映射
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;

	/** 为 true 时，该功能注册期间会同时把此映射上下文注册到玩家输入设置（EnhancedInputUserSettings） */
	UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;
};

/**
 * 向本地玩家的 EnhancedInput 系统添加 InputMappingContext
 * 前提：本地玩家已启用 EnhancedInput 系统
 *
 * 实现要点:
 * 1. 注册线（Registering）加载 IMC 使用 LoadSynchronous 同步加载，不依赖 AssetManager 包装
 * 2. 激活线（Activating）监听 UOmniInputComponent::NAME_BindInputsReady（输入绑定完成后广播），
 *    不监听 NAME_GameActorReady——
 *    它与 ExtensionAdded 同期到达，无增量信息
 * 3. AddInputMappingForPlayer 按 PC 查重（用 ControllersAddedTo 记录已挂过的 PC）：
 *    添加成功才记录，保证 Add/Remove 严格配对，防止 IMC 在 CountRegistrations 模式下多加少删残留
 */
UCLASS(MinimalAPI, meta = (DisplayName = "添加输入映射上下文"))
class UGameFeatureAction_AddInputContextMapping final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	UE_API virtual void OnGameFeatureRegistering() override;
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	UE_API virtual void OnGameFeatureUnregistering() override;
	//~End of UGameFeatureAction interface

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	UPROPERTY(EditAnywhere, Category="Input")
	TArray<FInputMappingContextAndPriority> InputMappings;

private:
	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** GameInstance 启动时注册 IMC 的委托句柄 */
	FDelegateHandle RegisterInputContextMappingsForGameInstanceHandle;

	/** 把持有的 IMC 注册到 Input 注册子系统，并绑定 GameInstance 启动与本地玩家增删事件 */
	UE_API void RegisterInputMappingContexts();

	/** 把持有的 IMC 注册到 Input 注册子系统（针对指定 GameInstance），GameInstance 启动时也会调用 */
	UE_API void RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);

	/** 把持有的 IMC 注册到 Input 注册子系统（针对指定本地玩家），本地玩家加入时也会调用 */
	UE_API void RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	/** 从 Input 注册子系统注销持有的 IMC，并解绑 GameInstance 启动与本地玩家增删事件 */
	UE_API void UnregisterInputMappingContexts();

	/** 从 Input 注册子系统注销持有的 IMC（针对指定 GameInstance） */
	UE_API void UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);

	/** 从 Input 注册子系统注销持有的 IMC（针对指定本地玩家），本地玩家移除时也会调用 */
	UE_API void UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	//~UGameFeatureAction_WorldActionBase interface
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~End of UGameFeatureAction_WorldActionBase interface

	UE_API void Reset(FPerContextData& ActiveData);
	UE_API void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	UE_API void AddInputMappingForPlayer(UPlayer* Player, FPerContextData& ActiveData);
	UE_API void RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData);
};

#undef UE_API
