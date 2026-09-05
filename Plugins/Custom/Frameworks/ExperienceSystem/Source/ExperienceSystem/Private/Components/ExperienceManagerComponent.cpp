// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExperienceManagerComponent.h"

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Data/ExperienceActionSet.h"
#include "Data/ExperienceDefinition.h"
#include "Engine/AssetManager.h"
#include "Logs/LogExperienceSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerComponent)

UExperienceManagerComponent::UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentExperience, SharedParams);
}

void UExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CloseExperience();
}

bool UExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (CurrentLoadState != EExperienceLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	return false;
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaded::FDelegate&& Delegate)
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

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate)
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

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate&& Delegate)
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

const UExperienceDefinition* UExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(CurrentLoadState == EExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

void UExperienceManagerComponent::SetCurrentExperience(const FPrimaryAssetId& ExperienceId)
{
	// 使用 ID 从资产管理器加载体验定义
	UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);
	// 获取 CDO 实例
	const UExperienceDefinition* Experience = GetDefault<UExperienceDefinition>(AssetClass);

	// 检查是否有效 且 未加载过,只能设置一次当前体验
	check(Experience != nullptr);
	check(CurrentExperience == nullptr);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CurrentExperience, this);
	CurrentExperience = Experience;

	// 开始加载体验
	StartExperienceLoad();
}

bool UExperienceManagerComponent::IsExperienceLoaded() const
{
	return (CurrentLoadState == EExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

void UExperienceManagerComponent::StartExperienceLoad()
{
	// 开始加载体验定义中的各种配置,资源

	// 1. 确保加载的前置条件,不能重复加载
	check(CurrentExperience != nullptr);
	check(CurrentLoadState == EExperienceLoadState::Unloaded);

	// Log一下当前体验和网络模式
	UE_LOG(LogExperienceSystem, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString());

	// 2. 进入加载的状态
	CurrentLoadState = EExperienceLoadState::Loading;

	// 3. 获取需要加载的资产ID列表
	UAssetManager& AssetManager = UAssetManager::Get();
	TSet<FPrimaryAssetId> BundleAssetList; // 需要加载的主资产ID
	TSet<FSoftObjectPath> RawAssetList; // 留着用于直接加载原始资源路径（当前为空，待实现）
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
	}

	//@TODO: 将此客户端/服务器内容集中到AssetManager中
	// 4. 根据不同的网络模式,决定需要加载那些资产(通过不同的FName来判断)
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

	// 5. 根据捆绑包状态,加载不同的资产,按照Bundles分类加载不同的资产
	// 	加载 Bundle 资产：通过 ChangeBundleStateForPrimaryAssets 异步加载 BundleAssetList 中的主资产，并应用 BundlesToLoad 标记。
	// 加载原始资产：预留逻辑（当前 RawAssetList 为空），通过 LoadAssetList 直接加载原始资源路径。
	// 合并加载句柄：如果同时存在 Bundle 和 Raw 资产加载，创建合并句柄统一管理加载完成事件。
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

	// 6. 当完成或者取消加载时,触发回调
	//创建回调委托：定义 OnAssetsLoadedDelegate，指向 OnExperienceLoadComplete（资产加载完成后的处理函数）。
	// 处理加载状态：
	// 如果资产已加载（或无加载句柄），直接执行回调。
	// 否则，绑定加载完成/取消事件到回调，确保加载结束后触发后续逻辑。
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnAssetLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// 资产已加载,直接执行回调
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

	// 7. 预留预加载逻辑
	// 	定义 PreloadAssetList 用于存储需要「预加载但不阻塞体验启动」的资产（当前未实现具体逻辑）。
	// 非阻塞加载：即使预加载未完成，体验启动流程也不会等待（通过 ChangeBundleStateForPrimaryAssets 异步加载）。
	// This set of assets gets preloaded, but we don't block the start of the experience based on it
	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void UExperienceManagerComponent::OnAssetLoadComplete()
{
	// 资产加载完成后,开始启动插件和执行的行为

	// 1. 前置检查
	check(CurrentLoadState == EExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	// Log一下状态
	UE_LOG(LogExperienceSystem, Log, TEXT("EXPERIENCE: OnAssetLoadComplete(CurrentExperience = %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString());

	//查找我们的GameFeaturePlugins的URL-过滤掉重复和没有有效映射的重复
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

	// 2. 将资产中所有的GameFeaturesToEnable添加到GameFeaturePluginURLs中
	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);
	for (const TObjectPtr<UExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
	}

	// 3. 根据URLs加载所有的插件
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		CurrentLoadState = EExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			// UExperienceManager::NotifyOfPluginActivation(PluginURL); 在编辑器中防止多PIE环境卸载插件
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

void UExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	NumGameFeaturePluginsLoading -= 1;
	if (NumGameFeaturePluginsLoading <= 0)
	{
		OnPluginsLoadComplete();
	}
}

namespace ExperienceManagerConsoleVariables
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

void UExperienceManagerComponent::OnPluginsLoadComplete()
{
	// 前置检查
	check(CurrentLoadState != EExperienceLoadState::Loaded);

	// 插入随机延迟进行测试（如果已配置）
	if (CurrentLoadState != EExperienceLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = ExperienceManagerConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;

			CurrentLoadState = EExperienceLoadState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnPluginsLoadComplete, DelaySecs, false);
			return;
		}
	}

	// 进入执行功能状态
	CurrentLoadState = EExperienceLoadState::ExecutingActions;

	// 功能的上下文
	FGameFeatureActivatingContext Context;

	// 设置上下文的世界上下文
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
				//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
				// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
				// but actually applying the results to actors is restricted to a specific world
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	// 根据函数 调用 功能 的所有函数
	ActivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) ActivateListOfActions(ActionSet->Actions);
	}

	// 进入到 已加载状态
	CurrentLoadState = EExperienceLoadState::Loaded;

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

void UExperienceManagerComponent::CloseExperience()
{
	// deactivate any features this experience loaded
	//@TODO: This should be handled FILO as well
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		// 防止多个PIE的情况下,直接就关闭了插件
		// if (UExperienceManager::RequestToDeactivatePlugin(PluginURL))
		UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
	}

	// 确保当前状态是已加载状态
	//@TODO: Ensure proper handling of a partially-loaded state too
	ensure(CurrentLoadState == EExperienceLoadState::Loaded);

	// 进入到退出状态
	CurrentLoadState = EExperienceLoadState::Deactivating;

	// Make sure we won't complete the transition prematurely if someone registers as a pauser but fires immediately
	NumExpectedPausers = INDEX_NONE;
	NumObservedPausers = 0;

	// 停用并卸载手动注册的功能
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
	for (const TObjectPtr<UExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr) DeactivateListOfActions(ActionSet->Actions);
	}

	NumExpectedPausers = Context.GetNumPausers();

	if (NumExpectedPausers > 0)
	{
		UE_LOG(LogExperienceSystem, Error, TEXT("Actions that have asynchronous deactivation aren't fully supported yet in  experiences"));
	}

	if (NumExpectedPausers == NumObservedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void UExperienceManagerComponent::OnAllActionsDeactivated()
{
	//@TODO: We actually only deactivated and didn't fully unload...
	CurrentLoadState = EExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
	//@TODO:	GEngine->ForceGarbageCollection(true);
}

void UExperienceManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	NumObservedPausers += 1;

	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}


void UExperienceManagerComponent::OnRep_CurrentExperience()
{
	//客户端收到后也开始加载
	StartExperienceLoad();
}
