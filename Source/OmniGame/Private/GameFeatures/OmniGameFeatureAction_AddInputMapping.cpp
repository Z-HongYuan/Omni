// Copyright © 2026 张鸿源. All Rights Reserved.

#include "GameFeatures/OmniGameFeatureAction_AddInputMapping.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Input/OmniInputManagerComponent.h"
#include "InputMappingContext.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameFeatureAction_AddInputMapping)

namespace
{
	// 用户设置没有引用计数，最后一个动作注销时才移除本动作体系拥有的注册。
	struct FSettingsRegistration
	{
		int32 Users = 0;
		bool bOwned = false;
	};

	TMap<TWeakObjectPtr<UEnhancedInputUserSettings>, TMap<const UInputMappingContext*, FSettingsRegistration>> SettingsRegistrations;

	void AcquireSettingsMapping(UEnhancedInputUserSettings* Settings, const UInputMappingContext* Mapping)
	{
		FSettingsRegistration& Registration = SettingsRegistrations.FindOrAdd(Settings).FindOrAdd(Mapping);
		if (Registration.Users++ == 0)
		{
			Registration.bOwned = !Settings->IsMappingContextRegistered(Mapping);
			if (Registration.bOwned) Settings->RegisterInputMappingContext(Mapping);
		}
	}

	void ReleaseSettingsMapping(TWeakObjectPtr<UEnhancedInputUserSettings> Settings, const UInputMappingContext* Mapping)
	{
		auto* Mappings = SettingsRegistrations.Find(Settings);
		FSettingsRegistration* Registration = Mappings ? Mappings->Find(Mapping) : nullptr;
		if (Registration && --Registration->Users == 0)
		{
			if (Settings.IsValid() && Registration->bOwned) Settings->UnregisterInputMappingContext(Mapping);
			Mappings->Remove(Mapping);
			if (Mappings->IsEmpty()) SettingsRegistrations.Remove(Settings);
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();
	if (bRegistering) return;
	bRegistering = true;
	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::RegisterGameInstance);
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		RegisterGameInstance(WorldContext.OwningGameInstance);
	}
}

void UOmniGameFeatureAction_AddInputMapping::OnGameFeatureUnregistering()
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);
	GameInstanceStartHandle.Reset();
	for (const TWeakObjectPtr<UGameInstance>& GameInstance : RegisteredGameInstances)
	{
		if (GameInstance.IsValid())
		{
			GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
			GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);
		}
	}
	RegisteredGameInstances.Reset();
	for (const auto& Player : RegisteredPlayers)
	{
		for (const UInputMappingContext* Mapping : Player.Value.Mappings) ReleaseSettingsMapping(Player.Value.Settings, Mapping);
	}
	RegisteredPlayers.Reset();
	bRegistering = false;
	LoadedMappings.Reset();
	Super::OnGameFeatureUnregistering();
}

void UOmniGameFeatureAction_AddInputMapping::RegisterGameInstance(UGameInstance* GameInstance)
{
	if (!GameInstance || RegisteredGameInstances.Contains(GameInstance)) return;
	RegisteredGameInstances.Add(GameInstance);
	GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::RegisterLocalPlayer);
	GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::UnregisterLocalPlayer);
	for (ULocalPlayer* Player : GameInstance->GetLocalPlayers()) RegisterLocalPlayer(Player);
}

void UOmniGameFeatureAction_AddInputMapping::RegisterLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer || RegisteredPlayers.Contains(LocalPlayer)) return;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	UEnhancedInputUserSettings* Settings = Subsystem ? Subsystem->GetUserSettings() : nullptr;
	if (!Settings) return; // 未开启用户设置不影响运行时映射。
	LoadMappings();
	FSettingsMappings& Data = RegisteredPlayers.Add(LocalPlayer);
	Data.Settings = Settings;
	for (const FOmniInputMappingContextAndPriority& Entry : InputMappings)
	{
		const UInputMappingContext* Mapping = Entry.InputMapping.Get();
		if (Entry.bRegisterWithSettings && Mapping && !Data.Mappings.Contains(Mapping))
		{
			AcquireSettingsMapping(Settings, Mapping);
			Data.Mappings.Add(Mapping);
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::UnregisterLocalPlayer(ULocalPlayer* LocalPlayer)
{
	FSettingsMappings SettingsData;
	if (RegisteredPlayers.RemoveAndCopyValue(LocalPlayer, SettingsData))
	{
		for (const UInputMappingContext* Mapping : SettingsData.Mappings) ReleaseSettingsMapping(SettingsData.Settings, Mapping);
	}
	// PC 可能尚未发 ReceiverRemoved，使用记录的子系统清理。
	for (auto& Context : ContextData)
	{
		for (auto It = Context.Value.Controllers.CreateIterator(); It; ++It)
		{
			UEnhancedInputLocalPlayerSubsystem* Subsystem = It.Value().Subsystem.Get();
			if (Subsystem && Subsystem->GetLocalPlayer() == LocalPlayer)
			{
				for (const UInputMappingContext* Mapping : It.Value().Mappings) Subsystem->RemoveMappingContext(Mapping);
				It.RemoveCurrent();
			}
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::LoadMappings()
{
	for (const FOmniInputMappingContextAndPriority& Entry : InputMappings)
	{
		if (Entry.InputMapping.IsNull()) continue;
		if (UInputMappingContext* Mapping = Entry.InputMapping.LoadSynchronous())
		{
			LoadedMappings.AddUnique(Mapping);
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("AddInputMapping: failed to load %s"), *Entry.InputMapping.ToString());
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	Reset(ActiveData);
	Super::OnGameFeatureActivating(Context);
}

void UOmniGameFeatureAction_AddInputMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	if (FPerContextData* ActiveData = ContextData.Find(Context))
	{
		Reset(*ActiveData);
		ContextData.Remove(Context);
	}
}

void UOmniGameFeatureAction_AddInputMapping::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer || !GameInstance) return;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	if (ActiveData.ExtensionHandles.Contains(GameInstance)) return;
	if (UGameFrameworkComponentManager* Manager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>())
	{
		auto Handler = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);
		ActiveData.ExtensionHandles.Add(GameInstance, Manager->AddExtensionHandler(APlayerController::StaticClass(), Handler));
	}
}

void UOmniGameFeatureAction_AddInputMapping::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* Controller = Cast<APlayerController>(Actor);
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (!Controller || !ActiveData) return;
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveInputMapping(Controller, *ActiveData);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UOmniInputManagerComponent::NAME_BindInputsNow)
	{
		AddInputMapping(Controller, *ActiveData);
	}
}

void UOmniGameFeatureAction_AddInputMapping::AddInputMapping(APlayerController* Controller, FPerContextData& ActiveData)
{
	ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!Subsystem || !Subsystem->GetPlayerInput() || ActiveData.Controllers.Contains(Controller)) return;
	if (bRegistering) RegisterLocalPlayer(LocalPlayer);
	LoadMappings();
	FPlayerMappings& Data = ActiveData.Controllers.Add(Controller);
	Data.Subsystem = Subsystem;
	for (const FOmniInputMappingContextAndPriority& Entry : InputMappings)
	{
		const UInputMappingContext* Mapping = Entry.InputMapping.Get();
		if (Mapping && !Data.Mappings.Contains(Mapping))
		{
			Subsystem->AddMappingContext(Mapping, Entry.Priority);
			Data.Mappings.Add(Mapping);
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::RemoveInputMapping(APlayerController* Controller, FPerContextData& ActiveData)
{
	FPlayerMappings Data;
	if (ActiveData.Controllers.RemoveAndCopyValue(Controller, Data))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Data.Subsystem.Get())
		{
			for (const UInputMappingContext* Mapping : Data.Mappings) Subsystem->RemoveMappingContext(Mapping);
		}
	}
}

void UOmniGameFeatureAction_AddInputMapping::Reset(FPerContextData& ActiveData)
{
	// 释放监听会同步发送 ExtensionRemoved，再处理已经失效的控制器记录。
	ActiveData.ExtensionHandles.Reset();
	for (const auto& Controller : ActiveData.Controllers)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Controller.Value.Subsystem.Get())
		{
			for (const UInputMappingContext* Mapping : Controller.Value.Mappings) Subsystem->RemoveMappingContext(Mapping);
		}
	}
	ActiveData.Controllers.Reset();
}

#if WITH_EDITOR
EDataValidationResult UOmniGameFeatureAction_AddInputMapping::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	TSet<FSoftObjectPath> Seen;
	for (const FOmniInputMappingContextAndPriority& Entry : InputMappings)
	{
		if (Entry.InputMapping.IsNull() || Seen.Contains(Entry.InputMapping.ToSoftObjectPath()))
		{
			Context.AddError(NSLOCTEXT("OmniInputMapping", "InvalidMapping", "Input mappings must be non-empty and unique."));
			Result = EDataValidationResult::Invalid;
		}
		Seen.Add(Entry.InputMapping.ToSoftObjectPath());
	}
	return Result;
}
#endif
