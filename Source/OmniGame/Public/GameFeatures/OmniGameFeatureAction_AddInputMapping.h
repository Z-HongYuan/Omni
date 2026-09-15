// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatures/OmniGameFeatureAction_WorldActionBase.h"
#include "OmniGameFeatureAction_AddInputMapping.generated.h"

#define UE_API OMNIGAME_API

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputUserSettings;
class ULocalPlayer;
class APlayerController;
struct FComponentRequestHandle;

USTRUCT(BlueprintType)
struct FOmniInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Priority = 0;

	// 注册阶段加入改键设置，不影响激活阶段是否应用 IMC。
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bRegisterWithSettings = true;
};

/**
 * 参考 Lyra AddInputContextMapping：注册/注销用户设置，激活/停用本地玩家 IMC。
 * 监听 InputManager 的 BindInputsNow 和 PC 扩展事件；不参与 ASC 生命周期。
 * 保留原版四个扩展事件，补齐控制器清理记录。
 * 多个动作共用 IMC 时，资产使用 CountRegistrations 且优先级保持一致。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Mapping (Omni)"))
class UOmniGameFeatureAction_AddInputMapping : public UOmniGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	UE_API virtual void OnGameFeatureRegistering() override;
	UE_API virtual void OnGameFeatureUnregistering() override;
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FOmniInputMappingContextAndPriority> InputMappings;

protected:
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	struct FPlayerMappings
	{
		TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem;
		TArray<const UInputMappingContext*> Mappings;
	};

	struct FPerContextData
	{
		TMap<TWeakObjectPtr<UGameInstance>, TSharedPtr<FComponentRequestHandle>> ExtensionHandles;
		TMap<TWeakObjectPtr<APlayerController>, FPlayerMappings> Controllers;
	};

	struct FSettingsMappings
	{
		TWeakObjectPtr<UEnhancedInputUserSettings> Settings;
		TArray<const UInputMappingContext*> Mappings;
	};

	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	void AddInputMapping(APlayerController* Controller, FPerContextData& ActiveData);
	void RemoveInputMapping(APlayerController* Controller, FPerContextData& ActiveData);
	void Reset(FPerContextData& ActiveData);
	void LoadMappings();
	void RegisterGameInstance(UGameInstance* GameInstance);
	void RegisterLocalPlayer(ULocalPlayer* LocalPlayer);
	void UnregisterLocalPlayer(ULocalPlayer* LocalPlayer);

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;
	TSet<TWeakObjectPtr<UGameInstance>> RegisteredGameInstances;
	TMap<TWeakObjectPtr<ULocalPlayer>, FSettingsMappings> RegisteredPlayers;
	FDelegateHandle GameInstanceStartHandle;
	bool bRegistering = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputMappingContext>> LoadedMappings;
};

#undef UE_API
