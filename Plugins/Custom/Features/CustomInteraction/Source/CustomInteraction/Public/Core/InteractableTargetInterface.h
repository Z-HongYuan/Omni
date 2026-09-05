// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InteractionOption.h"
#include "UObject/Interface.h"
#include "InteractableTargetInterface.generated.h"

#define UE_API CUSTOMINTERACTION_API

class IInteractableTargetInterface;
struct FInteractionOption;
struct FGameplayEventData;
struct FGameplayTag;
struct FInteractionQuery;

/** 交互选项构建器 */
class FInteractionOptionBuilder
{
public:
	FInteractionOptionBuilder(TScriptInterface<IInteractableTargetInterface> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions) :
		Scope(InterfaceTargetScope), Options(InteractOptions) { ; }

	void AddInteractionOption(const FInteractionOption& Option) const
	{
		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
		OptionEntry.InteractableTarget = Scope;
	}

private:
	TScriptInterface<IInteractableTargetInterface> Scope;
	TArray<FInteractionOption>& Options;
};

/**
 * 
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractableTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class IInteractableTargetInterface
{
	GENERATED_BODY()

public:
	/** 收集互动选项 */
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) = 0;

	/** 自定义交互事件数据 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { ; }
};

#undef UE_API
