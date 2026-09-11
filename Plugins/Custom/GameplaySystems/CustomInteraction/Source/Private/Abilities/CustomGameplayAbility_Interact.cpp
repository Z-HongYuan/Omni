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
			//TODO 这应该是一个响亮的警告。为什么我们要在一台永远无法处理互动的电脑上更新交互？
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

		// 允许目标自定义我们即将传递的事件数据，以防能力需要只有演员知道的自定义数据。
		FGameplayEventData Payload;
		Payload.EventTag = CustomInteractionTags::TAG_Ability_Interaction_Activate;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		// 如果需要，我们允许可交互的目标操作事件数据，比如墙上的某个按钮可能想指定一个门的演员来执行该能力，因此它可能会选择覆盖目标为门的演员。
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(CustomInteractionTags::TAG_Ability_Interaction_Activate, Payload);

		// 从负载上抓取目标演员，我们将用它作为交互的“化身”，而源 InteractableTarget 演员则作为拥有者角色。
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// 互动所需的演员信息。
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// 用事件标签触发该能力。
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			CustomInteractionTags::TAG_Ability_Interaction_Activate,
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);
	}
}
