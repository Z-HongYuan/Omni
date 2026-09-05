// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InteractionOption.h"
#include "UObject/Interface.h"
#include "InteractionInstigatorInterface.generated.h"

#define UE_API CUSTOMINTERACTION_API

struct FInteractionQuery;

/**
 *  实现此接口可让您在交互过程中添加仲裁器。例如，
 * 一些游戏会向用户呈现一个菜单，让他们选择想要执行的交互操作。这将允许你
 * 选择多个匹配项（假设您的 UGameplayAbility_Interact 子类生成了多个选项）。
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInteractionInstigatorInterface : public UInterface
{
	GENERATED_BODY()
};


class IInteractionInstigatorInterface
{
	GENERATED_BODY()

public:
	/** 如果有多个互动选项需要决定，将会被呼叫 */
	virtual FInteractionOption ChooseBestInteractionOption(const FInteractionQuery& InteractQuery, const TArray<FInteractionOption>& InteractOptions) = 0;
};

#undef UE_API
