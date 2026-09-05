// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/CustomAbilitySystemComponent.h"

#include "CustomAbilitySystemSettings.h"
#include "LogCustomAbilitySystem.h"
#include "Abilities/CustomGameplayAbility.h"
#include "Animation/TaggedAnimInstance.h"
#include "Data/CustomAbilitySystemTags.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "System/CustomAbilityTagRelationshipMapping.h"
#include "System/CustomGlobalAbilityManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomAbilitySystemComponent)

UCustomAbilitySystemComponent::UCustomAbilitySystemComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UCustomAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 从全局技能/效果管理器中移除ASC
	if (UCustomGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UCustomGlobalAbilityManager>(GetWorld()))
		GlobalAbilitySystem->UnregisterASC(this);

	Super::EndPlay(EndPlayReason);
}

void UCustomAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
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
				if (UCustomGameplayAbility* CustomAbilityInstance = Cast<UCustomGameplayAbility>(AbilityInstance))
				{
					// 回放时可能缺少技能实例
					CustomAbilityInstance->OnPawnAvatarSet();
				}
			}
		}

		// 只要拥有了Pawn才向全局系统注册。因为有些全局的效果可能需要一个Pawn。
		if (UCustomGlobalAbilityManager* GlobalAbilitySystem = UWorld::GetSubsystem<UCustomGlobalAbilityManager>(GetWorld()))
			GlobalAbilitySystem->RegisterASC(this);

		// 向动画实例注册ASC
		if (UTaggedAnimInstance* TaggedAnimInst = Cast<UTaggedAnimInstance>(ActorInfo->GetAnimInstance()))
			TaggedAnimInst->InitializeWithAbilitySystem(this);

		// 尝试激活技能(属于Spawn后立即执行)
		TryActivateAbilitiesOnSpawn();
	}
}

void UCustomAbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		// 过滤未激活技能
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UCustomGameplayAbility* AbilityCDO = Cast<UCustomGameplayAbility>(AbilitySpec.Ability);
		if (!AbilityCDO)
		{
			UE_LOG(LogCustomAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-CustomGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// 取消所有生成的实例。
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UCustomGameplayAbility* CustomAbilityInstance = CastChecked<UCustomGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(CustomAbilityInstance, AbilitySpec.Handle))
			{
				if (CustomAbilityInstance->CanBeCanceled())
				{
					CustomAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), CustomAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					UE_LOG(LogCustomAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *CustomAbilityInstance->GetName());
				}
			}
		}
	}
}

void UCustomAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UCustomGameplayAbility* CustomAbility, FGameplayAbilitySpecHandle Handle)
	{
		const ECustomAbilityActivationPolicy ActivationPolicy = CustomAbility->GetActivationPolicy();
		return ((ActivationPolicy == ECustomAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == ECustomAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UCustomAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
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

void UCustomAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
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

void UCustomAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(CustomAbilitySystemTags::TAG_Gameplay_AbilityInputBlocked))
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
				const UCustomGameplayAbility* CustomAbilityCDO = Cast<UCustomGameplayAbility>(AbilitySpec->Ability);
				if (CustomAbilityCDO && CustomAbilityCDO->GetActivationPolicy() == ECustomAbilityActivationPolicy::WhileInputActive)
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
					const UCustomGameplayAbility* CustomAbilityCDO = Cast<UCustomGameplayAbility>(AbilitySpec->Ability);
					if (CustomAbilityCDO && CustomAbilityCDO->GetActivationPolicy() == ECustomAbilityActivationPolicy::OnInputTriggered)
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

void UCustomAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UCustomAbilitySystemComponent::IsActivationGroupBlocked(ECustomAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case ECustomAbilityActivationGroup::Independent:
		// 独立能力从不被阻挡。
		bBlocked = false;
		break;

	case ECustomAbilityActivationGroup::Exclusive_Replaceable:
	case ECustomAbilityActivationGroup::Exclusive_Blocking:
		// 专属技能可以在没有阻挡的情况下激活。
		bBlocked = (ActivationGroupCounts[static_cast<uint8>(ECustomAbilityActivationGroup::Exclusive_Blocking)] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UCustomAbilitySystemComponent::AddAbilityToActivationGroup(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* CustomAbility)
{
	check(CustomAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case ECustomAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case ECustomAbilityActivationGroup::Exclusive_Replaceable:
	case ECustomAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(ECustomAbilityActivationGroup::Exclusive_Replaceable, CustomAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[static_cast<uint8>(ECustomAbilityActivationGroup::Exclusive_Replaceable)] + ActivationGroupCounts[static_cast<uint8>(ECustomAbilityActivationGroup::Exclusive_Blocking)];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogCustomAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UCustomAbilitySystemComponent::RemoveAbilityFromActivationGroup(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* CustomAbility)
{
	check(CustomAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void UCustomAbilitySystemComponent::CancelActivationGroupAbilities(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* IgnoreCustomAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreCustomAbility](const UCustomGameplayAbility* CustomAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((CustomAbility->GetActivationGroup() == Group) && (CustomAbility != IgnoreCustomAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UCustomAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UCustomAssetManager::GetSubclass(UCustomGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = GetDefault<UCustomAbilitySystemSettings>()->DynamicTagGameplayEffect;
	if (!DynamicTagGE)
	{
		UE_LOG(LogCustomAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *DynamicTagGE->GetName());
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (!Spec)
	{
		UE_LOG(LogCustomAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
		return;
	}

	Spec->DynamicGrantedTags.AddTag(Tag);

	ApplyGameplayEffectSpecToSelf(*Spec);
}

void UCustomAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UCustomAssetManager::GetSubclass(UCustomGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = GetDefault<UCustomAbilitySystemSettings>()->DynamicTagGameplayEffect;
	if (!DynamicTagGE)
	{
		UE_LOG(LogCustomAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *DynamicTagGE->GetName());
		return;
	}

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;

	RemoveActiveEffects(Query);
}

void UCustomAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

void UCustomAbilitySystemComponent::SetTagRelationshipMapping(UCustomAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UCustomAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping) TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
}

void UCustomAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UCustomGameplayAbility* CustomAbilityCDO = Cast<UCustomGameplayAbility>(AbilitySpec.Ability))
		{
			CustomAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UCustomAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
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

void UCustomAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
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

void UCustomAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UCustomGameplayAbility* CustomAbility = Cast<UCustomGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(CustomAbility->GetActivationGroup(), CustomAbility);
	}
}

void UCustomAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
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

void UCustomAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UCustomGameplayAbility* CustomAbility = Cast<UCustomGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(CustomAbility->GetActivationGroup(), CustomAbility);
	}
}

void UCustomAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags,
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

void UCustomAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: 应用任何特殊逻辑，如阻止输入或移动
}

void UCustomAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	UE_LOG(LogCustomAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const UCustomGameplayAbility* CustomAbility = Cast<const UCustomGameplayAbility>(Ability))
	{
		CustomAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

void UCustomAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}
