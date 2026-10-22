// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/ExtGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "LogAbilityExtension.h"
#include "MessageRouterManager.h"
#include "Abilities/ExtAbilityCost.h"
#include "Data/ExtAbilitySystemTags.h"
#include "Data/PhysicalMaterialWithTags.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)\
{\
if (!ensure(IsInstantiated()))\
	{\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());\
		return ReturnValue;\
	}\
}


UExtGameplayAbility::UExtGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EExtAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EExtAbilityActivationGroup::Independent;

	bLogCancelation = false;

	// ActiveCameraMode = nullptr; 暂时没有拓展摄像机模式的想法
}

UExtAbilitySystemComponent* UExtGameplayAbility::GetExtAbilitySystemComponentFromActorInfo() const
{
	return GetExtAbilitySystemComponentFromActorInfo<UExtAbilitySystemComponent>();
}

AController* UExtGameplayAbility::GetControllerFromActorInfo() const
{
	if (!CurrentActorInfo) return nullptr;

	if (AController* PC = CurrentActorInfo->PlayerController.Get()) return PC;

	// 沿 Owner 链查找 Controller，或通过 Pawn 取得 Controller。
	AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
	while (TestActor)
	{
		if (AController* C = Cast<AController>(TestActor)) return C;

		if (APawn* Pawn = Cast<APawn>(TestActor)) return Pawn->GetController();

		TestActor = TestActor->GetOwner();
	}

	return nullptr;
}

ACharacter* UExtGameplayAbility::GetCharacterFromActorInfo() const
{
	return GetCharacterFromActorInfo<ACharacter>();
}

void UExtGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& AbilitySpec) const
{
	// 仅处理尚未激活且采用 OnSpawn 策略的能力。
	if (ActorInfo && !AbilitySpec.IsActive() && (ActivationPolicy == EExtAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// Avatar 已 TearOff 或设置了剩余寿命时不自动激活，等待新的 Avatar。
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

bool UExtGameplayAbility::CanChangeActivationGroup(EExtAbilityActivationGroup NewGroup) const
{
	// 仅允许修改已激活的能力实例。
	if (!IsInstantiated() || !IsActive()) return false;

	// 当前已在目标组时无需更新。
	if (ActivationGroup == NewGroup) return true;

	UExtAbilitySystemComponent* ExtASC = GetExtAbilitySystemComponentFromActorInfo<UExtAbilitySystemComponent>();
	check(ExtASC);

	// 不能进入被阻塞的组，但当前能力本身占据阻塞组时允许切换。
	if ((ActivationGroup != EExtAbilityActivationGroup::Exclusive_Blocking) && ExtASC->IsActivationGroupBlocked(NewGroup)) return false;

	// 可替换组要求能力允许被取消。
	if ((NewGroup == EExtAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled()) return false;

	return true;
}

bool UExtGameplayAbility::ChangeActivationGroup(EExtAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup)) return false;


	if (ActivationGroup != NewGroup)
	{
		UExtAbilitySystemComponent* ExtASC = GetExtAbilitySystemComponentFromActorInfo<UExtAbilitySystemComponent>();
		check(ExtASC);

		ExtASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		ExtASC->AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

void UExtGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	// 文本只广播首个匹配结果，蒙太奇按所有匹配原因分别广播。
	bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FExtAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UMessageRouterManager& MessageSystem = UMessageRouterManager::Get(GetWorld());
				MessageSystem.BroadcastMessage(ExtAbilitySystemTags::TAG_Ability_SimpleTextFailure_Message, Message);
				bSimpleFailureFound = true;
			}
		}

		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FExtAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.AvatarActor = GetActorInfo().AvatarActor.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UMessageRouterManager& MessageSystem = UMessageRouterManager::Get(GetWorld());
			MessageSystem.BroadcastMessage(ExtAbilitySystemTags::TAG_Ability_PlayMontageFailure_Message, Message);
		}
	}
}

bool UExtGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                             FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return false;

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;

	//@TODO 设置标记关系后可能会删除
	UExtAbilitySystemComponent* ExtASC = CastChecked<UExtAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ExtASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags) OptionalRelevantTags->AddTag(ExtAbilitySystemTags::TAG_Ability_ActivateFail_ActivationGroup);
		return false;
	}

	return true;
}

void UExtGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// 可替换组必须保持可取消，才能被后续独占能力替换。
	if (!bCanBeCanceled && (ActivationGroup == EExtAbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogAbilityExtension, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UExtGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UExtGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UExtGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo) return false;

	// 所有额外消耗均须通过检查。
	for (const TObjectPtr<UExtAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UExtGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	// 仅在权威端检查目标数据的命中结果，供命中后才扣费的消耗使用。
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
		{
			if (UExtAbilitySystemComponent* ASC = Cast<UExtAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
			{
				FGameplayAbilityTargetDataHandle TargetData;
				ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
				{
					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
					{
						return true;
					}
				}
			}
		}

		return false;
	};

	// 按需检查一次命中结果，再结算各项额外消耗。
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (const TObjectPtr<UExtAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}
			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

FGameplayEffectContextHandle UExtGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Super::MakeEffectContext(Handle, ActorInfo);

	//自定义创建效果上下文

	// FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);
	//
	// FExtGameplayEffectContext* EffectContext = FExtGameplayEffectContext::ExtractEffectContext(ContextHandle);
	// check(EffectContext);
	//
	// check(ActorInfo);
	//
	// AActor* EffectCauser = nullptr;
	// const IExtAbilitySourceInterface* AbilitySource = nullptr;
	// float SourceLevel = 0.0f;
	// GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);
	//
	// UObject* SourceObject = GetSourceObject(Handle, ActorInfo);
	//
	// AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;
	//
	// EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	// EffectContext->AddInstigator(Instigator, EffectCauser);
	// EffectContext->AddSourceObject(SourceObject);
	//
	// return ContextHandle;
}

void UExtGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

	if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
		if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
			Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
}

bool UExtGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                            FGameplayTagContainer* OptionalRelevantTags) const
{
	// return Super::DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags);

	// 合并 ASC 标签关系表，并为死亡导致的标签阻塞补充失败原因。
	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// 先检查能力自身标签是否被 ASC 阻塞。
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const UExtAbilitySystemComponent* ExtASC = Cast<UExtAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// 追加关系表配置的必需标签与阻塞标签。
	if (ExtASC)
	{
		ExtASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	// 使用 ASC 当前标签检查阻塞条件和必需条件。
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(ExtAbilitySystemTags::TAG_Status_Death))
			{
				// 死亡状态下因标签阻塞被拒绝时，追加死亡失败原因。
				OptionalRelevantTags->AddTag(ExtAbilitySystemTags::TAG_Ability_ActivateFail_IsDead);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void UExtGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}
