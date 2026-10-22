// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniPawnInitializationComponent.generated.h"

#define UE_API OMNIGAME_API

/**
 * 玩家角色初始化组件，等待 PawnData、PlayerState 和控制关系就绪后请求绑定 ASC。
 * 在 DataAvailable -> DataInitialized 时完成一次性初始化，服务器同时恢复出生生命值。
 * EndPlay 注销本功能及状态监听；ASC 完整清理由 PawnExtension 负责，后续控制变化不在此重绑。
 */
UCLASS(MinimalAPI, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UOmniPawnInitializationComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniPawnInitializationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static UE_API const FName NAME_ActorFeatureName;
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;

private:
	bool IsPlayerStateReady() const;
};

#undef UE_API
