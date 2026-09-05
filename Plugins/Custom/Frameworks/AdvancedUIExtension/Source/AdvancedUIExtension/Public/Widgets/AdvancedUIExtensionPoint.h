// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Components/DynamicEntryBoxBase.h"
#include "Extension/AdvancedUIExtensionHelper.h"
#include "AdvancedUIExtensionPoint.generated.h"

#define UE_API ADVANCEDUIEXTENSION_API

class UExtensionLocalPlayer;

/**
 * 一个在布局中定义位置的插槽，后续可在该位置添加内容
 * 目前支持自动添加全局接收器和本地玩家上下文接收器,已移除自动添加来自于 CoreGame 的 PlayerState 上下文接收器
 */
UCLASS(MinimalAPI)
class UAdvancedUIExtensionPoint : public UDynamicEntryBoxBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, UObject*, DataItem);

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget*, Widget, UObject*, DataItem);

	UE_API UAdvancedUIExtensionPoint(const FObjectInitializer& ObjectInitializer);

	//~UWidget interface
	UE_API virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	UE_API virtual TSharedRef<SWidget> RebuildWidget() override;

#if WITH_EDITOR
	UE_API virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
	//~End of UWidget interface

private:
	// 释放拓展点(this)
	void ResetExtensionPoint();
	// 自动向拓展系统注册(this)和上下文为 LocalPlayer规则
	void RegisterExtensionPoint();
	// void RegisterExtensionPointForPlayerState(UExtensionLocalPlayer* LocalPlayer, APlayerState* PlayerState); 为了插件独立,暂时不需要以 PS 为上下文,因为以 PS 作为上下文需要 GameCore 引用,以创建对应的接收点
	void OnAddOrRemoveExtension(EUIExtensionAction Action, const FUIExtensionRequest& Request);

protected:
	/** 定义该延伸点的标签,用于识别 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/** 拓展点的匹配模式 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EUIExtensionPointMatch ExtensionPointTagMatch = EUIExtensionPointMatch::ExactMatch;

	// 拓展点 允许接收的数据类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TObjectPtr<UClass>> DataClasses;

	// 如果接受的是数据类 且 数据类允许接收 才会调用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnGetWidgetClassForData GetWidgetClassForData;

	// 如果接受的是数据类 且 数据类允许接收 才会调用 (需要 DataObj 中有效的 Widget ,否则不会执行)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnConfigureWidgetForData ConfigureWidgetForData;

	// 这个接收点的各种接受规则,其实是句柄
	TArray<FUIExtensionPointHandle> ExtensionPointHandles;

	FDelegateHandle PlayerStateChangedHandle;

	// 接收点规则与实际显示的 Widget 的映射
	UPROPERTY(Transient)
	TMap<FUIExtensionHandle, TObjectPtr<UUserWidget>> ExtensionMapping;
};

#undef UE_API
