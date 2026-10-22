// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InteractionOption.h"
#include "UObject/Interface.h"
#include "InteractionInstigatorInterface.generated.h"

#define UE_API CUSTOMINTERACTION_API

struct FInteractionQuery;

/** 提供多个交互选项之间的选择策略，由需要仲裁的调用方使用。 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractionInstigatorInterface : public UInterface
{
	GENERATED_BODY()
};


class IInteractionInstigatorInterface
{
	GENERATED_BODY()

public:
	/** 根据查询条件从候选列表中选择一个交互选项。 */
	virtual FInteractionOption ChooseBestInteractionOption(const FInteractionQuery& InteractQuery, const TArray<FInteractionOption>& InteractOptions) = 0;
};

#undef UE_API
