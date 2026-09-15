// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniPawnInitializationComponent.generated.h"

#define UE_API OMNIGAME_API

/**
 * 玩家角色初始化组件：等待 PawnData、PlayerState 和控制关系就绪，再由插件绑定 ASC；空占位 Pawn 不挂载。
 * 沿用 Lyra 5.8 HeroComponent 的注册/注销和一次性状态推进，在 DataAvailable -> DataInitialized 时请求插件绑定 ASC。
 * EndPlay 仅注销本功能及状态监听；ASC 完整清理由 PawnExtension 负责，不在这里自动重绑。
 * 与完整 HeroComponent 的差异：输入和相机由独立控制组件消费，ASC 初始化在服务器与客户端均参与。
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
