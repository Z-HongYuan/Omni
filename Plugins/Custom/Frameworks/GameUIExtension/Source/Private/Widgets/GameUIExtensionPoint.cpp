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

	// 仅校验实例配置，不要求 CDO 指定扩展点标签。
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
		AllowedDataClasses.Add(UUserWidget::StaticClass()); // 默认接受 UUserWidget 类。
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

		// 控件类扩展可直接创建对应控件。
		TSubclassOf<UUserWidget> WidgetClass(Cast<UClass>(Data));
		if (WidgetClass)
		{
			UUserWidget* Widget = CreateEntryInternal(WidgetClass);
			ExtensionMapping.Add(Request.ExtensionHandle, Widget);
		}
		// 数据扩展通过绑定的委托解析控件类。
		else if (DataClasses.Num() > 0)
		{
			if (GetWidgetClassForData.IsBound())
			{
				// 让接收方根据数据选择控件类。
				WidgetClass = GetWidgetClassForData.Execute(Data);

				// 控件创建成功后再交给接收方配置。
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

	// 按扩展句柄移除之前创建的控件。
	if (Action == EUIExtensionAction::Removed)
	{
		if (UUserWidget* Extension = ExtensionMapping.FindRef(Request.ExtensionHandle))
		{
			RemoveEntryInternal(Extension);
			ExtensionMapping.Remove(Request.ExtensionHandle);
		}
	}
}
