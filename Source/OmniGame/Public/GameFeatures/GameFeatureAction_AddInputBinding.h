// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"

#include "GameFeatureAction_AddInputBinding.generated.h"

#define UE_API OMNIGAME_API

class AActor;
class UCustomInputConfig;
class UInputMappingContext;
class UPlayer;
class APlayerController;
struct FComponentRequestHandle;

/**
 * 给本地玩家的 Pawn 追加输入配置（能力输入绑定）的 GameFeatureAction
 *
 * 职责：
 * - 功能激活时对所有 APawn 注册扩展处理器；Pawn 就绪
 *   （或 UOmniInputComponent 广播 NAME_BindInputsReady）时，把 InputConfigs
 *   交给 Pawn 上的 UOmniInputComponent::AddAdditionalInputConfig 绑定能力输入
 * - 功能反激活时调用 RemoveAdditionalInputConfig 解绑并清除记录
 *
 * 注意：
 * - 只负责"能力输入"（InputTag→ASC 转发）的绑定，不注册 IMC——IMC 映射由
 *   GameFeatureAction_AddInputContextMapping 负责，两者各管一半避免重复
 * - Bot 与专用服务器上没有本地玩家，自动跳过
 *
 * 与 Lyra 的差异:
 * 1. ULyraInputConfig → UCustomInputConfig；ULyraHeroComponent → UOmniInputComponent
 * 2. 就绪事件监听 UOmniInputComponent::NAME_BindInputsReady，对应 Lyra 的 HeroComponent::NAME_BindInputsNow
 * 3. 移除线是真实解绑（Lyra 的 HeroComponent::RemoveAdditionalInputConfig 是 @TODO 空实现）
 */
UCLASS(MinimalAPI, meta = (DisplayName = "添加输入绑定"))
class UGameFeatureAction_AddInputBinding final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	// 要追加绑定的输入配置列表
	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UCustomInputConfig>> InputConfigs;

private:
	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APawn>> PawnsAddedTo;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase interface
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~End of UGameFeatureAction_WorldActionBase interface

	UE_API void Reset(FPerContextData& ActiveData);
	UE_API void HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	UE_API void AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData);
	UE_API void RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData);
};

#undef UE_API
