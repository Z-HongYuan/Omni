// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExpPawnExtensionComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExtAbilitySystemTags.h"
#include "Data/ExpSystemTags.h"
#include "Data/ExpPawnData.h"
#include "Logs/LogExpSystem.h"
#include "Net/UnrealNetwork.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpPawnExtensionComponent)

UExpPawnExtensionComponent::UExpPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 初始化由事件推动，不需要 Tick。
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UExpPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PawnData);
}

void UExpPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("ExpPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(UExpPawnExtensionComponent::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one PawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// 确认组件只挂载于 Pawn 且没有重复实例后，注册初始化功能。
	RegisterInitStateFeature();
}

void UExpPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听同一 Actor 上所有初始化功能的状态变化。
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// 先进入 Spawned，再尝试推进后续状态。
	ensure(TryToChangeInitState(ExpSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UExpPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();

	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

const FName UExpPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

bool UExpPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == ExpSystemTags::TAG_InitState_Spawned)
	{
		if (Pawn) return true;
	}

	if (CurrentState == ExpSystemTags::TAG_InitState_Spawned && DesiredState == ExpSystemTags::TAG_InitState_DataAvailable)
	{
		if (!PawnData) return false;

		// 权威端和本地控制端需要 Controller；模拟代理不以此为前置条件。
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		if (bHasAuthority || bIsLocallyControlled)
		{
			if (!GetController<AController>()) return false;
		}

		return true;
	}

	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		// 所有功能都达到 DataAvailable 后才能跨过共同初始化屏障。
		return Manager->HaveAllFeaturesReachedInitState(Pawn, ExpSystemTags::TAG_InitState_DataAvailable);
	}

	if (CurrentState == ExpSystemTags::TAG_InitState_DataInitialized && DesiredState == ExpSystemTags::TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UExpPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		// 此处只提供初始化屏障，具体数据初始化由其他功能负责。
	}
}

void UExpPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 其他功能达到 DataAvailable 时重新检查屏障，全部就绪后才能推进。
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == ExpSystemTags::TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UExpPawnExtensionComponent::CheckDefaultInitialization()
{
	// 先推动其他初始化功能，再检查本组件依赖是否满足。
	CheckDefaultInitializationForImplementers();

	// 按已注册状态链依次推进，不回退状态。
	ContinueInitStateChain(ExpSystemTags::ComponentStateChain);
}

void UExpPawnExtensionComponent::SetPawnData(const UExpPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	// 仅权威端写入 PawnData，由复制通知客户端。
	if (!Pawn->HasAuthority()) return;

	// PawnData 只设置一次，避免初始化后替换配置。
	if (PawnData)
	{
		UE_LOG(LogExpSystemPawnData, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	// 请求网络更新并重新检查初始化条件。
	Pawn->ForceNetUpdate();
	CheckDefaultInitialization();
}

void UExpPawnExtensionComponent::CallOrRegister_AbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UExpPawnExtensionComponent::Register_AbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UExpPawnExtensionComponent::InitializeAbilitySystem(UExtAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	// ASC 未变时不重复绑定和广播。
	if (AbilitySystemComponent == InASC) return;

	// 先解除本组件已有的 ASC 绑定。
	if (AbilitySystemComponent) UninitializeAbilitySystem();


	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogExpSystem, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogExpSystem, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// 客户端复制顺序可能使新 Pawn 先于旧 Pawn 清理完成，需要先解除旧 Avatar 绑定。
		ensure(!ExistingAvatar->HasAuthority());

		if (UExpPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	// 将 PawnData 的能力标签关系映射应用到 ASC。
	if (ensure(PawnData))
	{
		InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
	}

	OnAbilitySystemInitialized.Broadcast();
}

void UExpPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent) return;

	// 只清理仍以当前 Pawn 为 Avatar 的 ASC，避免旧 Pawn 影响新的绑定。
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		// 与死亡技能共用豁免标签；这里只取消运行中的技能，不移除授予。
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(ExtAbilitySystemTags::TAG_Ability_Behavior_AvoidDeathClear);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// ASC 已无 Owner 时清空 ActorInfo，不只清空 Avatar。
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

void UExpPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UExpPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UExpPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UExpPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}
