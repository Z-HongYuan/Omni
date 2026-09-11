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
	// 从全局技能/效果管理器中移除ASC
	if (UExtGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UExtGlobalAbilityManager>(GetWorld()))
		GlobalAbilitySystem->UnregisterASC(this);

	Super::EndPlay(EndPlayReason);
}

void UExtAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// 检查ActorInfo
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	// 是否是新的AvatarPawn
	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// 通知所有可激活技能新角色已设定
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
					// 回放时可能缺少技能实例
					ExtAbilityInstance->OnPawnAvatarSet();
				}
			}
		}

		// 只要拥有了Pawn才向全局系统注册。因为有些全局的效果可能需要一个Pawn。
		if (UExtGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UExtGlobalAbilityManager>(GetWorld()))
			GlobalAbilitySystem->RegisterASC(this);

		// 向动画实例注册ASC
		if (UTaggedAnimInstance* TaggedAnimInst = Cast<UTaggedAnimInstance>(ActorInfo->GetAnimInstance()))
			TaggedAnimInst->InitializeWithAbilitySystem(this);

		// 尝试激活技能(属于Spawn后立即执行)
		TryActivateAbilitiesOnSpawn();
	}
}

void UExtAbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		// 过滤未激活技能
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

		// 取消所有生成的实例。
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

	//@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

	// 1. 处理所有在按住输入时激活的能力。
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

	// 2. 处理所有在按下输入时激活的能力。
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// 当前技能已激活,所以再次传递一下输入事件
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

	/*
	 * 一次性（同一帧内）批量处理所有由"按下"和"长按"触发的技能激活。
	 * 如果按顺序逐个处理（先激活技能，再发输入事件），会出现一个 bug：某个按钮的长按判定满足了 → 激活了技能 → 但同一帧这按钮还是按着的 → 于是又立刻往这个刚激活的技能里塞了一个输入事件。
	 * 把"激活"和"发输入事件"放在一起批量判断
	 */
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// 3. 处理所有在本帧释放输入的能力
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// 当前能力已激活,所以再次传递一下输入事件
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// 清除缓存输入,保存 Held 输入缓存
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
		// 独立能力从不被阻挡。
		bBlocked = false;
		break;

	case EExtAbilityActivationGroup::Exclusive_Replaceable:
	case EExtAbilityActivationGroup::Exclusive_Blocking:
		// 专属技能可以在没有阻挡的情况下激活。
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
		// Independent abilities do not cancel any other abilities.
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

	// 我们不支持UGameplayAbility：：bReplicateInputDirectly。
	// 请改用复制事件，以便WaitInputPress功能任务正常工作。
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 调用InputPressed事件。这里没有重复。如果有人在监听，他们可能会将InputPressed事件复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UExtAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// 我们不支持UGameplayAbility：：bReplicateInputDirectly。
	// 请改用复制事件，以便WaitInputRelease功能任务能够工作。
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 调用InputRelease事件。这里没有重复。如果有人在监听，他们可能会将InputRelease事件复制到服务器。
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
	//通过 TagRelationshipMapping 数据资产进行扩展。
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// 使用映射将能力标签展开为块并取消标签
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
