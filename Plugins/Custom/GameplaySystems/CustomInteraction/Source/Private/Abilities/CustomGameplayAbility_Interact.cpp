// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/CustomGameplayAbility_Interact.h"
#include "AbilitySystemComponent.h"
#include "CustomInteractionTags.h"
#include "IndicatorManagerComponent.h"
#include "Core/InteractableTargetInterface.h"
#include "Data/InteractionHelper.h"
#include "Tasks/AbilityTask_GrantNearbyInteraction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomGameplayAbility_Interact)

UCustomGameplayAbility_Interact::UCustomGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActivationPolicy = EExtAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UCustomGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantNearbyInteraction* Task = UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(this, InteractChannel, InteractionScanRange, InteractionScanRate);
		Task->ReadyForActivation();
	}
}

void UCustomGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
{
	if (AController* PC = GetControllerFromActorInfo())
	{
		if (UIndicatorManagerComponent* IndicatorManager = UIndicatorManagerComponent::GetComponent(PC))
		{
			for (UIndicatorDescriptorDataObj* Indicator : Indicators)
			{
				IndicatorManager->RemoveIndicator(Indicator);
			}
			Indicators.Reset();

			for (const FInteractionOption& InteractionOption : InteractiveOptions)
			{
				AActor* InteractableTargetActor = UInteractionHelper::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

				TSoftClassPtr<UUserWidget> InteractionWidgetClass = InteractionOption.InteractionWidgetClass.IsNull() ? DefaultInteractionWidgetClass : InteractionOption.InteractionWidgetClass;

				UIndicatorDescriptorDataObj* Indicator = NewObject<UIndicatorDescriptorDataObj>();
				Indicator->IndicatorDescriptorParameter.DataObject = InteractableTargetActor;
				Indicator->IndicatorDescriptorParameter.SceneComponent = InteractableTargetActor->GetRootComponent();
				Indicator->IndicatorDescriptorParameter.IndicatorWidgetClass = InteractionWidgetClass;
				IndicatorManager->AddIndicator(Indicator);

				Indicators.Add(Indicator);
			}
		}
		else
		{
			// TODO: 控制器缺少指示器管理组件时，补充诊断以说明交互提示无法显示。
		}
	}

	CurrentOptions = InteractiveOptions;
}

void UCustomGameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo())
	{
		const FInteractionOption& InteractionOption = CurrentOptions[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = UInteractionHelper::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		// 先填入默认事件上下文，再交给交互目标补充或覆盖。
		FGameplayEventData Payload;
		Payload.EventTag = CustomInteractionTags::TAG_Ability_Interaction_Activate;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		// 交互入口可以改写实际作用目标，例如由按钮将目标改为门。
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(CustomInteractionTags::TAG_Ability_Interaction_Activate, Payload);

		// 原交互目标作为 Owner，事件中的最终目标作为能力 Avatar。
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// 使用选项指定的能力系统构建本次触发上下文。
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// 将事件数据交给选项对应的能力。
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			CustomInteractionTags::TAG_Ability_Interaction_Activate,
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);
	}
}
