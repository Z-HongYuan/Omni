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

	// 从拥有链中寻找Pawn/控制器
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
	// 如果激活政策在生成时尝试激活。
	if (ActorInfo && !AbilitySpec.IsActive() && (ActivationPolicy == EExtAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// 如果化身演员被销毁或快销毁，别尝试激活，直到我们拿到新的。
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
	// 禁止 非实例化,未激活 修改
	if (!IsInstantiated() || !IsActive()) return false;

	// 避免重复修改
	if (ActivationGroup == NewGroup) return true;

	UExtAbilitySystemComponent* ExtASC = GetExtAbilitySystemComponentFromActorInfo<UExtAbilitySystemComponent>();
	check(ExtASC);

	// 这个技能如果被阻挡，就无法改变群体（除非是它自己在阻挡）。
	if ((ActivationGroup != EExtAbilityActivationGroup::Exclusive_Blocking) && ExtASC->IsActivationGroupBlocked(NewGroup)) return false;

	// 如果无法取消，这个能力就无法被替换。
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
	// 失败处理
	// 当失败时，尝试通过MessageManager发送一个失败消息。
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
	// 如果技能可以替换，这个技能就无法阻挡取消。
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

	// 确认我们能负担任何额外费用
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

	// 用于判断该能力是否真的命中目标（因为部分消耗仅在成功尝试时花费）
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

	// 应用额外消耗
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

	// 专门版本，用于通过ASC处理死亡排除和能力标签扩展
	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// 检查一下该能力的标签是否被封锁
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const UExtAbilitySystemComponent* ExtASC = Cast<UExtAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// 扩展我们的能力标签，以添加更多必需的blocked标签
	if (ExtASC)
	{
		ExtASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	// 查看该能力所需的Blocked标签
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(ExtAbilitySystemTags::TAG_Status_Death))
			{
				// 如果玩家已经死亡，因为标签被阻挡而被拒绝，给出反馈
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
