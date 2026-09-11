// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Widgets/GameUIExtensionPoint.h"

// #include "CoreGameplay/CustomLocalPlayer.h"
#include "Editor/WidgetCompilerLog.h"
#include "Extension/GameUIExtensionManager.h"
#include "Misc/UObjectToken.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIExtensionPoint)

UGameUIExtensionPoint::UGameUIExtensionPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UGameUIExtensionPoint::ReleaseSlateResources(bool bReleaseChildren)
{
	ResetExtensionPoint();

	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> UGameUIExtensionPoint::RebuildWidget()
{
	if (!IsDesignTime() && ExtensionPointTag.IsValid())
	{
		ResetExtensionPoint();
		RegisterExtensionPoint();

		// PlayerStateChangedHandle = GetOwningLocalPlayer<UExtLocalPlayer>()->CallAndRegister_OnPlayerStateSet(
		// 	UExtLocalPlayer::FPlayerStateSetDelegate::FDelegate::CreateUObject(this, &UGameUIExtensionPoint::RegisterExtensionPointForPlayerState)
		// );
	}

	if (IsDesignTime())
	{
		auto GetExtensionPointText = [this]()
		{
			return FText::Format(NSLOCTEXT("UGameUIExtensionPoint", "DesignTime_ExtensionPointLabel", "Extension Point\n{0}"), FText::FromName(ExtensionPointTag.GetTagName()));
		};

		TSharedRef<SOverlay> MessageBox = SNew(SOverlay);

		MessageBox->AddSlot()
		          .Padding(5.0f)
		          .HAlign(HAlign_Center)
		          .VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text_Lambda(GetExtensionPointText)
		];

		return MessageBox;
	}
	else
	{
		return Super::RebuildWidget();
	}
}

#if WITH_EDITOR
void UGameUIExtensionPoint::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// 我们不在乎CDO是否有特定的标签。
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!ExtensionPointTag.IsValid())
		{
			TSharedRef<FTokenizedMessage> Message = CompileLog.Error(FText::Format(
				NSLOCTEXT("UGameUIExtensionPoint", "UUIExtensionPointWidget_NoTag", "{0} has no ExtensionPointTag specified - All extension points must specify a tag so they can be located."), FText::FromString(GetName())));
			Message->AddToken(FUObjectToken::Create(this));
		}
	}
}
#endif

void UGameUIExtensionPoint::ResetExtensionPoint()
{
	ResetInternal();

	ExtensionMapping.Reset();
	for (FUIExtensionPointHandle& Handle : ExtensionPointHandles)
	{
		Handle.Unregister();
	}
	ExtensionPointHandles.Reset();
	PlayerStateChangedHandle.Reset();
}

void UGameUIExtensionPoint::RegisterExtensionPoint()
{
	if (UGameUIExtensionManager* ExtensionSubsystem = GetWorld()->GetSubsystem<UGameUIExtensionManager>())
	{
		TArray<UClass*> AllowedDataClasses;
		AllowedDataClasses.Add(UUserWidget::StaticClass()); //默认允许 UserWidget
		AllowedDataClasses.Append(DataClasses);

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPoint(
			ExtensionPointTag, ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContext(
			ExtensionPointTag, GetOwningLocalPlayer(), ExtensionPointTagMatch, AllowedDataClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
		));
	}
}

// void UGameUIExtensionPoint::RegisterExtensionPointForPlayerState(UExtLocalPlayer* LocalPlayer, APlayerState* PlayerState)
// {
// 	if (UGameUIExtensionManager* ExtensionSubsystem = GetWorld()->GetSubsystem<UGameUIExtensionManager>())
// 	{
// 		TArray<UClass*> AllowedDataClasses;
// 		AllowedDataClasses.Add(UUserWidget::StaticClass());
// 		AllowedDataClasses.Append(DataClasses);
//
// 		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContext(
// 			ExtensionPointTag, PlayerState, ExtensionPointTagMatch, AllowedDataClasses,
// 			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension)
// 		));
// 	}
// }

void UGameUIExtensionPoint::OnAddOrRemoveExtension(EUIExtensionAction Action, const FUIExtensionRequest& Request)
{
	if (Action == EUIExtensionAction::Added)
	{
		UObject* Data = Request.Data;

		// 如果数据完全指向 Widget 的情况 直接创建就行
		TSubclassOf<UUserWidget> WidgetClass(Cast<UClass>(Data));
		if (WidgetClass)
		{
			UUserWidget* Widget = CreateEntryInternal(WidgetClass);
			ExtensionMapping.Add(Request.ExtensionHandle, Widget);
		}
		// 如果不是的话,就看看是否能使用这个 DataObj
		else if (DataClasses.Num() > 0)
		{
			if (GetWidgetClassForData.IsBound())
			{
				// 通过委托函数来获取 Data 中包含的控件类
				WidgetClass = GetWidgetClassForData.Execute(Data);

				// 如果控件有效,添加后调用另一个函数用于配置这个控件
				if (WidgetClass)
				{
					if (UUserWidget* Widget = CreateEntryInternal(WidgetClass))
					{
						ExtensionMapping.Add(Request.ExtensionHandle, Widget);
						ConfigureWidgetForData.ExecuteIfBound(Widget, Data);
					}
				}
			}
		}
	}

	// 移除流程
	if (Action == EUIExtensionAction::Removed)
	{
		if (UUserWidget* Extension = ExtensionMapping.FindRef(Request.ExtensionHandle))
		{
			RemoveEntryInternal(Extension);
			ExtensionMapping.Remove(Request.ExtensionHandle);
		}
	}
}
