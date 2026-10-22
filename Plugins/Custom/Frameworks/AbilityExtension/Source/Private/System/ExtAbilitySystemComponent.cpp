// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/ExtAbilitySystemComponent.h"

#include "ExtAbilitySystemSettings.h"
#include "LogAbilityExtension.h"
#include "Abilities/ExtGameplayAbility.h"
#include "Animation/TaggedAnimInstance.h"
#include "Data/ExtAbilitySystemTags.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "System/ExtAbilityTagRelationshipMapping.h"
#include "System/ExtGlobalAbilityManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtAbilitySystemComponent)

UExtAbilitySystemComponent::UExtAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UExtAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 结束生命周期时注销 ASC，并撤销管理器授予的全局能力和效果。
	if (UExtGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UExtGlobalAbilityManager>(GetWorld()))
		GlobalAbilitySystem->UnregisterASC(this);

	Super::EndPlay(EndPlayReason);
}

void UExtAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// 初始化前需要有效的 ActorInfo 和 Owner。
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	// 只在 Avatar 切换为新的 Pawn 时执行后续初始化。
	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// 通知已有扩展能力实例，新的 Pawn Avatar 已就绪。
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced,
			           TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
			PRAGMA_ENABLE_DEPRECATION_WARNINGS

			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				if (UExtGameplayAbility* ExtAbilityInstance = Cast<UExtGameplayAbility>(AbilityInstance))
				{
					// 只向扩展能力实例转发 Pawn 就绪通知。
					ExtAbilityInstance->OnPawnAvatarSet();
				}
			}
		}

		// Pawn 就绪后才注册全局能力和效果，避免其使用尚未设置的 Avatar。
		if (UExtGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UExtGlobalAbilityManager>(GetWorld()))
			GlobalAbilitySystem->RegisterASC(this);

		// 将 ASC 标签绑定到支持标签映射的动画实例。
		if (UTaggedAnimInstance* TaggedAnimInst = Cast<UTaggedAnimInstance>(ActorInfo->GetAnimInstance()))
			TaggedAnimInst->InitializeWithAbilitySystem(this);

		// 尝试激活采用 OnSpawn 策略的能力。
		TryActivateAbilitiesOnSpawn();
	}
}

void UExtAbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		// 未激活的能力无需取消。
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UExtGameplayAbility* AbilityCDO = Cast<UExtGameplayAbility>(AbilitySpec.Ability);
		if (!AbilityCDO)
		{
			UE_LOG(LogAbilityExtension, Error, TEXT("CancelAbilitiesByFunc: Non-ExtGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 逐个检查实例，仅取消满足条件且允许取消的实例。
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UExtGameplayAbility* ExtAbilityInstance = CastChecked<UExtGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(ExtAbilityInstance, AbilitySpec.Handle))
			{
				if (ExtAbilityInstance->CanBeCanceled())
				{
					ExtAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), ExtAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					UE_LOG(LogAbilityExtension, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *ExtAbilityInstance->GetName());
				}
			}
		}
	}
}

void UExtAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UExtGameplayAbility* ExtAbility, FGameplayAbilitySpecHandle Handle)
	{
		const EExtAbilityActivationPolicy ActivationPolicy = ExtAbility->GetActivationPolicy();
		return ((ActivationPolicy == EExtAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EExtAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UExtAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability) continue;
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UExtAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability) continue;
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UExtAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(ExtAbilitySystemTags::TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// TODO: 评估是否可在这些循环中使用 FScopedServerAbilityRPCBatcher 合并 RPC。

	// 收集按住期间需要尝试激活的能力。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UExtGameplayAbility* ExtAbilityCDO = Cast<UExtGameplayAbility>(AbilitySpec->Ability);
				if (ExtAbilityCDO && ExtAbilityCDO->GetActivationPolicy() == EExtAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// 记录按下状态，并向已经激活的能力转发输入事件。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// 已激活的能力接收本次按下事件。
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UExtGameplayAbility* ExtAbilityCDO = Cast<UExtGameplayAbility>(AbilitySpec->Ability);
					if (ExtAbilityCDO && ExtAbilityCDO->GetActivationPolicy() == EExtAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	// 收集完按住和按下输入后统一激活，避免新激活能力在同一帧又收到一次按下事件。
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// 处理本帧释放事件，并更新能力的输入状态。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// 已激活的能力接收本次释放事件。
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// 清空本帧边沿事件，保留持续按住的输入。
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UExtAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UExtAbilitySystemComponent::IsActivationGroupBlocked(EExtAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EExtAbilityActivationGroup::Independent:
		// 独立能力不受激活组阻塞，其他激活条件仍然有效。
		bBlocked = false;
		break;

	case EExtAbilityActivationGroup::Exclusive_Replaceable:
	case EExtAbilityActivationGroup::Exclusive_Blocking:
		// 存在独占阻塞能力时，其他独占能力不能激活。
		bBlocked = (ActivationGroupCounts[static_cast<uint8>(EExtAbilityActivationGroup::Exclusive_Blocking)] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UExtAbilitySystemComponent::AddAbilityToActivationGroup(EExtAbilityActivationGroup Group, UExtGameplayAbility* ExtAbility)
{
	check(ExtAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EExtAbilityActivationGroup::Independent:
		// 独立能力不因激活组规则取消其他能力。
		break;

	case EExtAbilityActivationGroup::Exclusive_Replaceable:
	case EExtAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EExtAbilityActivationGroup::Exclusive_Replaceable, ExtAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[static_cast<uint8>(EExtAbilityActivationGroup::Exclusive_Replaceable)] + ActivationGroupCounts[static_cast<uint8>(EExtAbilityActivationGroup::Exclusive_Blocking)];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogAbilityExtension, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UExtAbilitySystemComponent::RemoveAbilityFromActivationGroup(EExtAbilityActivationGroup Group, UExtGameplayAbility* ExtAbility)
{
	check(ExtAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void UExtAbilitySystemComponent::CancelActivationGroupAbilities(EExtAbilityActivationGroup Group, UExtGameplayAbility* IgnoreExtAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreExtAbility](const UExtGameplayAbility* ExtAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((ExtAbility->GetActivationGroup() == Group) && (ExtAbility != IgnoreExtAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UExtAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UExtAssetManager::GetSubclass(UExtGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = GetDefault<UExtAbilitySystemSettings>()->DynamicTagGameplayEffect;
	if (!DynamicTagGE)
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *DynamicTagGE->GetName());
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (!Spec)
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
		return;
	}

	Spec->DynamicGrantedTags.AddTag(Tag);

	ApplyGameplayEffectSpecToSelf(*Spec);
}

void UExtAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UExtAssetManager::GetSubclass(UExtGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = GetDefault<UExtAbilitySystemSettings>()->DynamicTagGameplayEffect;
	if (!DynamicTagGE)
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *DynamicTagGE->GetName());
		return;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;

	RemoveActiveEffects(Query);
}

void UExtAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

void UExtAbilitySystemComponent::SetTagRelationshipMapping(UExtAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UExtAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping) TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
}

void UExtAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UExtGameplayAbility* ExtAbilityCDO = Cast<UExtGameplayAbility>(AbilitySpec.Ability))
		{
			ExtAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UExtAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// 使用通用复制事件通知 WaitInputPress，输入配置不应依赖 bReplicateInputDirectly。
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 此处触发本地 InputPressed 事件，由监听任务按需复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UExtAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// 使用通用复制事件通知 WaitInputRelease，输入配置不应依赖 bReplicateInputDirectly。
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 此处触发本地 InputReleased 事件，由监听任务按需复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void UExtAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UExtGameplayAbility* ExtAbility = Cast<UExtGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(ExtAbility->GetActivationGroup(), ExtAbility);
	}
}

void UExtAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UExtAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UExtGameplayAbility* ExtAbility = Cast<UExtGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(ExtAbility->GetActivationGroup(), ExtAbility);
	}
}

void UExtAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags,
                                                                const FGameplayTagContainer& CancelTags)
{
	// 保留调用方标签，并追加关系表中的规则。
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// 追加关系表中配置的阻塞和取消标签。
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: 应用任何特殊逻辑，如阻止输入或移动
}

void UExtAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: 应用任何特殊逻辑，如阻止输入或移动
}

void UExtAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	UE_LOG(LogAbilityExtension, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const UExtGameplayAbility* ExtAbility = Cast<const UExtGameplayAbility>(Ability))
	{
		ExtAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

void UExtAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}
