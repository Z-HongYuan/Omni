// Copyright © 2026 张鸿源. All Rights Reserved.


#include "MessagingSystem/MessagingManager.h"
#include "AdvancedUISettings.h"
#include "Engine/GameInstance.h"
#include "LogAdvancedUI.h"
#include "MessagingSystem/DialogWidgetBase.h"
#include "System/AdvancedUIGameplayTags.h"
#include "System/UIManager.h"
#include "System/UIPolicy.h"
#include "Widgets/GameRootLayoutWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AdvancedUISettings)
#include UE_INLINE_GENERATED_CPP_BY_NAME(MessagingManager)

void UMessagingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAdvancedUISettings* Settings = GetDefault<UAdvancedUISettings>();
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
	//一般是 通过本地玩家获取到其代表的 RootWidget ,然后推送函数对应属性的Widget到栈中,并且传递数据源 (DialogDescriptor);
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UUIManager* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UUIManager>();
	if (!UIManager) return;

	UUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
	if (!Policy) return;

	UGameRootLayoutWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer);
	if (!RootLayout) return;

	if (!DialogDescriptor)
	{
		// 数据源为空(例如 ShowDialogCustom 传入了空 Descriptor),直接以空结果结束
		UE_LOG(LogAdvancedUI, Warning, TEXT("UMessagingManager::ShowDialogInternal: Dialog descriptor is null for tag [%s]"), *InWidgetTag.ToString());
		ResultCallback.ExecuteIfBound(FGameplayTag::EmptyTag);
		return;
	}

	const TSubclassOf<UDialogWidgetBase> DialogClassPtr = DialogClassMap.FindRef(InWidgetTag);
	if (!DialogClassPtr)
	{
		// 该 Tag 未在开发者设置中配置对应对话框类,直接以空结果结束,避免推送空类导致崩溃
		UE_LOG(LogAdvancedUI, Warning, TEXT("UMessagingManager::ShowDialogInternal: No dialog widget class configured for tag [%s]"), *InWidgetTag.ToString());
		ResultCallback.ExecuteIfBound(FGameplayTag::EmptyTag);
		return;
	}

	RootLayout->PushWidgetToLayerStack<UDialogWidgetBase>(
		AdvancedUITags::TAG_AdvancedUI_UIStack_Modal,
		DialogClassPtr,
		[DialogDescriptor, ResultCallback](UDialogWidgetBase& DialogWidget)
		{
			DialogWidget.SetupDialog(DialogDescriptor, ResultCallback);
		});

	// RootLayout->PushWidgetToLayerStackAsync<UDialogWidgetBase>(
	// 	AdvancedUITags::TAG_AdvancedUI_UIStack_Modal,
	// 	true,
	// 	DialogClassPtr,
	// 	[DialogDescriptor, ResultCallback](EAsyncWidgetPushState PushState, UDialogWidgetBase* DialogWidget)
	// 	{
	// 		if (PushState == EAsyncWidgetPushState::Canceled || !DialogWidget)
	// 		{
	// 			// 加载被取消或失败,以空结果结束
	// 			ResultCallback.ExecuteIfBound(FGameplayTag::EmptyTag);
	// 			return;
	// 		}
	//
	// 		if (PushState == EAsyncWidgetPushState::Initialize)
	// 		{
	// 			DialogWidget->SetupDialog(DialogDescriptor, ResultCallback);
	// 		}
	// 	});
}
