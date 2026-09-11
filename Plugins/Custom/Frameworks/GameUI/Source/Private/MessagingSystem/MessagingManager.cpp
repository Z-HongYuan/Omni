// Copyright © 2026 张鸿源. All Rights Reserved.


#include "MessagingSystem/MessagingManager.h"
#include "GameUISettings.h"
#include "Engine/GameInstance.h"
#include "LogGameUI.h"
#include "MessagingSystem/DialogWidgetBase.h"
#include "System/GameUIGameplayTags.h"
#include "System/GameUIManager.h"
#include "System/GameUIPolicy.h"
#include "Widgets/GameUIRootWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUISettings)
#include UE_INLINE_GENERATED_CPP_BY_NAME(MessagingManager)

void UMessagingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UGameUISettings* Settings = GetDefault<UGameUISettings>();
	for (const auto& DialogSoftClass : Settings->DialogSoftClasses)
	{
		DialogClassMap.Add(DialogSoftClass.Key, DialogSoftClass.Value.LoadSynchronous());
	}
}

void UMessagingManager::Deinitialize()
{
	Super::Deinitialize();

	DialogClassMap.Reset();
}

bool UMessagingManager::ShouldCreateSubsystem(UObject* Outer) const
{
	//保持继承链单例
	UGameInstance* GameInstance = CastChecked<ULocalPlayer>(Outer)->GetGameInstance();
	if (GameInstance && !GameInstance->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 只有在其他地方没有定义覆盖实现的情况下才创建实例
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UMessagingManager::ShowDialogInternal(FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* DialogDescriptor, FDialogMessagingResultDelegate ResultCallback)
{
	// 展示前的所有失败出口均返回空结果，避免异步调用方一直等待。
	const auto Fail = [&ResultCallback, InWidgetTag](const TCHAR* Reason)
	{
		UE_LOG(LogGameUI, Warning, TEXT("UMessagingManager::ShowDialogInternal: 无法展示 [%s]：%s"), *InWidgetTag.ToString(), Reason);
		ResultCallback.ExecuteIfBound(FGameplayTag::EmptyTag);
	};

	if (!DialogDescriptor)
	{
		Fail(TEXT("对话框描述为空"));
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UGameInstance* GameInstance = LocalPlayer ? LocalPlayer->GetGameInstance() : nullptr;
	UGameUIManager* GameUIManager = GameInstance ? GameInstance->GetSubsystem<UGameUIManager>() : nullptr;
	UGameUIPolicy* Policy = GameUIManager ? GameUIManager->GetCurrentUIPolicy() : nullptr;
	UGameUIRootWidget* RootLayout = Policy ? Policy->GetRootLayoutWidget(LocalPlayer) : nullptr;
	if (!RootLayout)
	{
		Fail(TEXT("本地玩家的 UI 策略或根布局尚未就绪"));
		return;
	}

	const TSubclassOf<UDialogWidgetBase> DialogClass = DialogClassMap.FindRef(InWidgetTag);
	if (!DialogClass || DialogClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		Fail(TEXT("未配置可实例化的对话框类"));
		return;
	}

	UDialogWidgetBase* Dialog = RootLayout->PushWidgetToLayerStack<UDialogWidgetBase>(
		GameUITags::TAG_GameUI_UIStack_Modal,
		DialogClass,
		[DialogDescriptor, ResultCallback](UDialogWidgetBase& DialogWidget)
		{
			DialogWidget.SetupDialog(DialogDescriptor, ResultCallback);
		});
	if (!Dialog) Fail(TEXT("Modal 层不存在或控件创建失败"));
}
