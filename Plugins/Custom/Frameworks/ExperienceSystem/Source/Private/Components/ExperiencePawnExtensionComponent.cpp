// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExperiencePawnExtensionComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExperienceSystemTags.h"
#include "Data/ExperiencePawnData.h"
#include "Logs/LogExperienceSystem.h"
#include "Net/UnrealNetwork.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePawnExtensionComponent)

UExperiencePawnExtensionComponent::UExperiencePawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 不需要Tick更新
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UExperiencePawnExtensionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PawnData);
}

void UExperiencePawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(UExperiencePawnExtensionComponent::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one PawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// 将次插件注册到初始化状态系统中,前面的都是检查条件
	RegisterInitStateFeature();
}

void UExperiencePawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听其他所有的组件变化
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// 直接尝试进入第一个状态,并且推动一下其他状态
	ensure(TryToChangeInitState(ExperienceSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 注销 ASC
	UninitializeAbilitySystem();

	// 注销初始化状态功能
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

// 初始化状态使用的判断标志
const FName UExperiencePawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

bool UExperiencePawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	// 前置条件
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	// 从 None 进入 Spawned 状态
	if (!CurrentState.IsValid() && DesiredState == ExperienceSystemTags::TAG_InitState_Spawned)
	{
		// 只要Pawn存在，就认为是已生成
		if (Pawn) return true;
	}

	// 从 Spawned 进入 DataAvailable 状态
	if (CurrentState == ExperienceSystemTags::TAG_InitState_Spawned && DesiredState == ExperienceSystemTags::TAG_InitState_DataAvailable)
	{
		// 需要PawnData有效
		if (!PawnData) return false;

		// 在客户端或者服务器 都需要有控制器
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		if (bHasAuthority || bIsLocallyControlled)
		{
			if (!GetController<AController>()) return false;
		}

		return true;
	}

	// 从 DataAvailable 进入 DataInitialized 状态
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		//如果所有功能都有可用数据，则转换为初始化
		//相当于其他的所有组件都需要进入到 DataAvailable
		return Manager->HaveAllFeaturesReachedInitState(Pawn, ExperienceSystemTags::TAG_InitState_DataAvailable);
	}

	// 从 DataInitialized 进入 GameplayReady 状态
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataInitialized && DesiredState == ExperienceSystemTags::TAG_InitState_GameplayReady)
	{
		// 无条件进入
		return true;
	}

	return false;
}

void UExperiencePawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		// 不做任何处理,这个组件仅是保证其他组件的流程时机
		// 其他组件将会处理
	}
}

void UExperiencePawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 在其他任何组件进入 DataAvailable 状态时，检查是否需要推进下一个状态,这样就确保了所有组件都有可用数据，才会进入 DataInitialized 状态
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == ExperienceSystemTags::TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UExperiencePawnExtensionComponent::CheckDefaultInitialization()
{
	//在检查我们的进度之前，请尝试开发我们可能依赖的任何其他功能
	CheckDefaultInitializationForImplementers();

	// 按照状态链依次尝试推进
	ContinueInitStateChain(ExperienceSystemTags::ComponentStateChain);
}

void UExperiencePawnExtensionComponent::SetPawnData(const UExperiencePawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	// 必须 在权威中设置
	if (!Pawn->HasAuthority()) return;

	// 如果已经设置了PawnData,则不允许重复设置
	if (PawnData)
	{
		UE_LOG(LogExperienceSystemPawnData, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	// 强制网络更新,并推进初始化状态链
	Pawn->ForceNetUpdate();
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::CallOrRegister_AbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// 没有被绑定过，才添加
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UExperiencePawnExtensionComponent::Register_AbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// 没有被绑定过，才添加
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UExperiencePawnExtensionComponent::InitializeAbilitySystem(UCustomAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	// 前置条件
	check(InASC);
	check(InOwnerActor);

	// 没有改变的话就不做任何事情
	if (AbilitySystemComponent == InASC) return;

	// 先清理一遍 旧的 ASC
	if (AbilitySystemComponent) UninitializeAbilitySystem();


	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogExperienceSystem, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogExperienceSystem, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		//已经有一枚棋子充当ASC的化身，所以我们需要把它踢出去
		//客户滞后会导致 NewPawn在OldPawn移除之前生成并控制了
		ensure(!ExistingAvatar->HasAuthority());

		if (UExperiencePawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	// 设置 ASC 的能力关系映射
	if (ensure(PawnData))
	{
		InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
	}

	OnAbilitySystemInitialized.Broadcast();
}

void UExperiencePawnExtensionComponent::UninitializeAbilitySystem()
{
	// 前置检查
	if (!AbilitySystemComponent) return;

	// 确保 ASC 的化身就是 当前Pawn
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		// 如果 技能附带有特定 Tag 将不会移除能力
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(ExperienceSystemTags::TAG_Ability_Behavior_AvoidDeathClear);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// 如果 ASC 没有有效信息,那么要清除所有的演员信息
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

void UExperiencePawnExtensionComponent::HandleControllerChanged()
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

void UExperiencePawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UExperiencePawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}
