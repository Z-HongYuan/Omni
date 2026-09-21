// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Character/OmniPawnInitializationComponent.h"

#include "Component/ExtHealthComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExpSystemTags.h"
#include "GameFramework/Controller.h"
#include "Gameplay/ExpPlayerState.h"
#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniPawnInitializationComponent)

UOmniPawnInitializationComponent::UOmniPawnInitializationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOmniPawnInitializationComponent::OnRegister()
{
	Super::OnRegister();

	if (ensure(GetPawn<APawn>())) RegisterInitStateFeature();
}

void UOmniPawnInitializationComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UExpPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(ExpSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniPawnInitializationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 与 Lyra HeroComponent 一致：同时移除功能注册和状态监听，ASC 由 PawnExtension 清理。
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

const FName UOmniPawnInitializationComponent::NAME_ActorFeatureName("PawnInitialization");

bool UOmniPawnInitializationComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == ExpSystemTags::TAG_InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!Extension)
	{
		return false;
	}

	if (CurrentState == ExpSystemTags::TAG_InitState_Spawned && DesiredState == ExpSystemTags::TAG_InitState_DataAvailable)
	{
		// PawnData 由 PawnExtension 检查；输入控制组件的就绪条件通过同一功能屏障参与。
		return IsPlayerStateReady();
	}
	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		return IsPlayerStateReady() && Manager->HasFeatureReachedInitState(Pawn, UExpPawnExtensionComponent::NAME_ActorFeatureName, ExpSystemTags::TAG_InitState_DataInitialized);
	}
	if (CurrentState == ExpSystemTags::TAG_InitState_DataInitialized && DesiredState == ExpSystemTags::TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UOmniPawnInitializationComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AExpPlayerState* PlayerState = GetPlayerState<AExpPlayerState>();
		if (!ensure(Pawn && PlayerState))
		{
			return;
		}

		if (UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			// ASC 属于 PS；插件统一处理旧 Avatar、ActorInfo、关系映射及初始化广播。
			Extension->InitializeAbilitySystem(PlayerState->GetExtAbilitySystemComponent(), PlayerState);

			if (Pawn->HasAuthority())
			{
				// 只在新 Pawn 的数据初始化阶段恢复生命，普通 ASC 重绑不经过此入口。
				if (UExtHealthComponent* HealthComponent = UExtHealthComponent::FindHealthComponent(Pawn))
				{
					HealthComponent->InitializeHealthForSpawn();
				}
			}

			UE_LOG(LogOmniGame, Log, TEXT("OmniPawn ASC initialized: Pawn=%s, PlayerState=%s"), *GetPathNameSafe(Pawn), *GetPathNameSafe(PlayerState));
		}
	}
}

void UOmniPawnInitializationComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UExpPawnExtensionComponent::NAME_ActorFeatureName
		&& Params.FeatureState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniPawnInitializationComponent::CheckDefaultInitialization()
{
	// 与 Lyra 一致，状态链不回退；后续控制变化不在这里触发 ASC 重绑。
	ContinueInitStateChain(ExpSystemTags::ComponentStateChain);
}

bool UOmniPawnInitializationComponent::IsPlayerStateReady() const
{
	const APawn* Pawn = GetPawn<APawn>();
	const AExpPlayerState* PlayerState = Pawn ? Pawn->GetPlayerState<AExpPlayerState>() : nullptr;
	if (!PlayerState || !PlayerState->GetExtAbilitySystemComponent())
	{
		return false;
	}

	// 模拟代理通常没有 Controller；服务器与自主代理需等待控制器/PlayerState 配对。
	if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const AController* Controller = Pawn->GetController();
		return Controller && Controller->PlayerState == PlayerState && PlayerState->GetOwner() == Controller;
	}

	return true;
}
