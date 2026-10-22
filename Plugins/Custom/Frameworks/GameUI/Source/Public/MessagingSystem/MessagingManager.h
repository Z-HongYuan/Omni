// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "DialogWidgetDescriptorBase.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SubclassOf.h"
#include "MessagingManager.generated.h"

#define UE_API GAMEUI_API

class UDialogWidgetBase;
DECLARE_DELEGATE_OneParam(FDialogMessagingResultDelegate, FGameplayTag);

/** 管理本地玩家的对话框类映射，并在 Modal 层显示对话框。 */
UCLASS(MinimalAPI, Config = Game)
class UMessagingManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UMessagingManager() { ; }

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 按标签映射将对话框推入 Modal 层；展示失败时立即回调空标签。 */
	UE_API virtual void ShowDialogInternal(FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* DialogDescriptor, FDialogMessagingResultDelegate ResultCallback = FDialogMessagingResultDelegate());

private:
	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<UDialogWidgetBase>> DialogClassMap;
};

#undef UE_API
