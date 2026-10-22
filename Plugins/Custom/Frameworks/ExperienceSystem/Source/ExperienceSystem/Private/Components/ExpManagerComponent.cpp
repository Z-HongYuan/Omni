// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExpManagerComponent.h"

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Data/ExpActionSet.h"
#include "Data/ExpDefinition.h"
#include "Engine/AssetManager.h"
#include "Gameplay/ExpPluginCountManager.h"
#include "Logs/LogExpSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpManagerComponent)

UExpManagerComponent::UExpManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UExpManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentExperience, SharedParams);
}

void UExpManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CloseExperience();
}

bool UExpManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (CurrentLoadState != EExpLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	return false;
}

void UExpManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(FExpLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void UExpManagerComponent::CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void UExpManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(FExpLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

const UExpDefinition* UExpManagerComponent::GetCurrentExperienceChecked() const
{
	check(CurrentLoadState == EExpLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

void UExpManagerComponent::SetCurrentExperience(const FPrimaryAssetId& ExperienceId)
{
	// 按主资产 ID 同步加载体验定义，后续引用资产再走异步加载。
	UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	// 定义为普通数据资产，直接读取资产对象，不加载蓝图类或访问 CDO。
	const UExpDefinition* Experience = Cast<UExpDefinition>(AssetPath.TryLoad());

	// 只允许从空体验开始加载，不能直接覆盖已有体验。
	checkf(Experience != nullptr, TEXT("Experience %s at %s must be an ExpDefinition data asset; disable Has Blueprint Classes in AssetManager settings."),
	       *ExperienceId.ToString(), *AssetPath.ToString());
	check(CurrentExperience == nullptr);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentExperience, this);
	CurrentExperience = Experience;

	StartExperienceLoad();
}

bool UExpManagerComponent::IsExperienceLoaded() const
{
	return (CurrentLoadState == EExpLoadState::Loaded) && (CurrentExperience != nullptr);
}

void UExpManagerComponent::StartExperienceLoad()
{

	check(CurrentExperience != nullptr);
	check(CurrentLoadState == EExpLoadState::Unloaded);

	UE_LOG(LogExpSystem, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString());

	CurrentLoadState = EExpLoadState::Loading;

	// 收集体验定义和共享 ActionSet 的主资产 ID。
	UAssetManager& AssetManager = UAssetManager::Get();
	TSet<FPrimaryAssetId> BundleAssetList; // 需要加载 Bundle 的主资产 ID。
	TSet<FSoftObjectPath> RawAssetList; // 直接资源路径的预留列表，当前未填充。
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UExpActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
	}

	// TODO: 将客户端和服务器的 Bundle 选择集中到 AssetManager。
	// 编辑器加载客户端和服务器 Bundle；独立进程按网络角色选择。
	TArray<FName> BundlesToLoad;
	// BundlesToLoad.Add(FBundles::Equipped);
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);
	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	// 分别发起 Bundle 与直接路径加载，必要时合并句柄，以统一后续回调。
	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}
	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({BundleLoadHandle, RawLoadHandle});
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	// 已完成或没有句柄时直接推进；取消也调用 OnAssetLoadComplete，当前没有独立的取消状态。
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// 无需等待加载时执行后续回调。
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
		{
			OnAssetsLoadedDelegate.ExecuteIfBound();
		}));
	}

	// 预留不阻塞体验启动的预加载列表，当前尚未确定需要填充的资产。
	TSet<FPrimaryAssetId> PreloadAssetList;
	// TODO: 确定不阻塞启动的预加载资产。
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void UExpManagerComponent::OnAssetLoadComplete()
{

	check(CurrentLoadState == EExpLoadState::Loading);
	check(CurrentExperience != nullptr);

	UE_LOG(LogExpSystem, Log, TEXT("EXPERIENCE: OnAssetLoadComplete(CurrentExperience = %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString());

	// 将插件名称解析为 URL 并去重；映射失败时报告检查错误并跳过。
	GameFeaturePluginURLs.Reset();

	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnAssetLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}

		// 		// Add in our extra plugin
		// 		if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
		// 		{
		// 			FString PluginURL;
		// 			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
		// 			{
		// 				GameFeaturePluginURLs.AddUnique(PluginURL);
		// 			}
		// 		}
	};

	// 合并体验定义和共享 ActionSet 的插件依赖。
	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);
	for (const TObjectPtr<UExpActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
	}

	// 按 URL 加载并激活插件，全部回调返回后再执行 Action。
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		CurrentLoadState = EExpLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			UExpPluginCountManager::NotifyOfPluginActivation(PluginURL);
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL,
				FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete)
			);
		}
	}
	else
	{
		OnPluginsLoadComplete();
	}
}

void UExpManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// 当前按回调次数推进，未检查 Result 的成功状态；失败处理仍待完善。
	NumGameFeaturePluginsLoading -= 1;
	if (NumGameFeaturePluginsLoading <= 0)
	{
		OnPluginsLoadComplete();
	}
}

namespace ExpManagerConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("ExperienceManager.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT("This value (in seconds) will be added as a delay of load completion of the experience (along with the random value ExperienceManager.chaos.ExperienceDelayLoad.RandomSecs)"),
		ECVF_Default);

	static float ExperienceLoadRandomDelayRange = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("ExperienceManager.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT("A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value ExperienceManager.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default);

	static float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
}

void UExpManagerComponent::OnPluginsLoadComplete()
{
	check(CurrentLoadState != EExpLoadState::Loaded);

	// 按配置插入一次加载测试延迟，计时回调不再次延迟。
	if (CurrentLoadState != EExpLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = ExpManagerConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;

			CurrentLoadState = EExpLoadState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnPluginsLoadComplete, DelaySecs, false);
			return;
		}
	}

	CurrentLoadState = EExpLoadState::ExecutingActions;

	FGameFeatureActivatingContext Context;

	// 限制 Action 激活的目标世界，避免影响同一进程中的其他 PIE 世界。
	if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				// TODO: 检查注册和加载阶段不带世界上下文时的多世界 PIE 隔离。
				// 注册与加载作用于进程，激活通过 Context 限定目标世界。
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	// 先执行体验 Action，再执行共享 ActionSet。
	ActivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UExpActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) ActivateListOfActions(ActionSet->Actions);
	}

	CurrentLoadState = EExpLoadState::Loaded;

	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();

	// Apply any necessary scalability settings
	// #if !UE_SERVER
	// 	USettingsLocal::Get()->OnExperienceLoaded();
	// #endif
}

void UExpManagerComponent::CloseExperience()
{
	// 请求停用本体验激活的插件，多个 PIE 世界共享时由计数管理器协调。
	// TODO: 按激活的逆序停用插件。
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		// 只有最后一个请求者退出时才允许停用共用插件。
		if (UExpPluginCountManager::RequestToDeactivatePlugin(PluginURL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
		}
	}

	// TODO: 完善体验尚未加载完成时的退出处理。
	ensure(CurrentLoadState == EExpLoadState::Loaded);

	CurrentLoadState = EExpLoadState::Deactivating;

	// 先用无效期望数防止停用回调同步完成时过早结束，注册完成后再比较等待数量。
	NumExpectedPausers = INDEX_NONE;
	NumObservedPausers = 0;

	// 停用并注销手动注册的 Action，完整资产卸载仍未实现。
	FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });

	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action)
			{
				Action->OnGameFeatureDeactivating(Context);
				Action->OnGameFeatureUnregistering();
			}
		}
	};

	DeactivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UExpActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) DeactivateListOfActions(ActionSet->Actions);
	}

	NumExpectedPausers = Context.GetNumPausers();

	if (NumExpectedPausers > 0)
	{
		UE_LOG(LogExpSystem, Error, TEXT("Actions that have asynchronous deactivation aren't fully supported yet in  experiences"));
	}

	if (NumExpectedPausers == NumObservedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void UExpManagerComponent::OnAllActionsDeactivated()
{
	// TODO: 当前仅完成停用，仍需实现完整卸载。
	CurrentLoadState = EExpLoadState::Unloaded;
	CurrentExperience = nullptr;
	//@TODO:	GEngine->ForceGarbageCollection(true);
}

void UExpManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	NumObservedPausers += 1;

	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}


void UExpManagerComponent::OnRep_CurrentExperience()
{
	// 客户端收到体验定义后，在本机启动相同加载流程。
	StartExperienceLoad();
}
