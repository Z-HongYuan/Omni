// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "Core/InteractionOption.h"
#include "CustomGameplayAbility_Interact.generated.h"

#define UE_API CUSTOMINTERACTION_API


class UIndicatorDescriptorDataObj;
/**
 * 
 */
UCLASS(MinimalAPI, Abstract)
class UCustomGameplayAbility_Interact : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UCustomGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable)
	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<FInteractionOption> CurrentOptions;

	//UI 小部件
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptorDataObj>> Indicators;

protected:
	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRange = 500;

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> InteractChannel;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};

#undef UE_API
