// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Components/DynamicEntryBoxBase.h"
#include "Extension/GameUIExtensionHelper.h"
#include "GameUIExtensionPoint.generated.h"

#define UE_API GAMEUIEXTENSION_API

class UExtLocalPlayer;

/**
 * 在布局中预留动态内容位置，按扩展通知创建或移除控件。
 * 自动注册无上下文及当前 LocalPlayer 的扩展点，释放 Slate 资源时注销。
 */
UCLASS(MinimalAPI)
class UGameUIExtensionPoint : public UDynamicEntryBoxBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, UObject*, DataItem);

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget*, Widget, UObject*, DataItem);

	UE_API UGameUIExtensionPoint(const FObjectInitializer& ObjectInitializer);

	//~UWidget interface
	UE_API virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	UE_API virtual TSharedRef<SWidget> RebuildWidget() override;

#if WITH_EDITOR
	UE_API virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
	//~End of UWidget interface

private:
	/** 清空生成的控件，并注销此控件持有的扩展点。 */
	void ResetExtensionPoint();
	/** 注册无上下文和当前 LocalPlayer 两种接收条件。 */
	void RegisterExtensionPoint();
	// void RegisterExtensionPointForPlayerState(UExtLocalPlayer* LocalPlayer, APlayerState* PlayerState); 为了插件独立,暂时不需要以 PS 为上下文,因为以 PS 作为上下文需要 GameCore 引用,以创建对应的接收点
	void OnAddOrRemoveExtension(EUIExtensionAction Action, const FUIExtensionRequest& Request);

protected:
	/** 用于定位此扩展点的标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/** 扩展点的标签匹配模式。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EUIExtensionPointMatch ExtensionPointTagMatch = EUIExtensionPointMatch::ExactMatch;

	/** 除 UUserWidget 类以外允许接收的数据类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TObjectPtr<UClass>> DataClasses;

	/** 数据扩展到达时解析其控件类；须绑定此委托才能创建控件。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnGetWidgetClassForData GetWidgetClassForData;

	/** 数据扩展对应的控件成功创建后，使用数据配置该控件。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnConfigureWidgetForData ConfigureWidgetForData;

	/** 此控件注册的扩展点句柄，释放时逐个注销。 */
	TArray<FUIExtensionPointHandle> ExtensionPointHandles;

	FDelegateHandle PlayerStateChangedHandle;

	/** 扩展句柄到已创建控件的映射，用于响应移除通知。 */
	UPROPERTY(Transient)
	TMap<FUIExtensionHandle, TObjectPtr<UUserWidget>> ExtensionMapping;
};

#undef UE_API
