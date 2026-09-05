// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "DialogWidgetDescriptorBase.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SubclassOf.h"
#include "MessagingManager.generated.h"

#define UE_API ADVANCEDUI_API

class UDialogWidgetBase;
DECLARE_DELEGATE_OneParam(FDialogMessagingResultDelegate, FGameplayTag);

/**
 * 对话系统管理器,用于显示对话框,蓝图内一般是使用 Task 节点
 */
UCLASS(MinimalAPI, Config = Game)
class UMessagingManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UMessagingManager() { ; }

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 使用对话数据推送指定 Tag 的控件到 Modal 层了
	UE_API virtual void ShowDialogInternal(FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* DialogDescriptor, FDialogMessagingResultDelegate ResultCallback = FDialogMessagingResultDelegate());

private:
	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<UDialogWidgetBase>> DialogClassMap;
};

#undef UE_API
