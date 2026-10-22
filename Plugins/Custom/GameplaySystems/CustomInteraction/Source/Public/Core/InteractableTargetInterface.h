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

/** 将交互选项加入调用方数组，并统一绑定当前交互目标。 */
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

/** 由交互目标收集可用选项，并按需修改触发能力的事件数据。 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractableTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class IInteractableTargetInterface
{
	GENERATED_BODY()

public:
	/** 根据查询条件收集当前可提供的交互选项。 */
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) = 0;

	/** 在能力触发前修改事件数据，例如将事件目标改为实际作用对象。 */
	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { ; }
};

#undef UE_API
