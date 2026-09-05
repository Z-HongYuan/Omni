// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayCueManager.h"
#include "CustomGameplayCueManager.generated.h"

#define UE_API CUSTOMABILITYSYSTEM_API

enum class ECueEditorLoadMode
{
	// 一次性全加载Cue
	LoadUpfront,

	// 游戏中异步预加载，编辑器全加载
	PreloadAsCuesAreReferenced_GameOnly,

	// 总是异步预加载
	PreloadAsCuesAreReferenced
};

/**
 * 同步加载全禁、异步加载全开。确保加载操作永远不会阻塞游戏线程。
 */
UCLASS(MinimalAPI)
class UCustomGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	UCustomGameplayCueManager() { ; }
	static UCustomGameplayCueManager* Get();

	//~UGameplayCueManager interface
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;
	//~End of UGameplayCueManager interface

	static void DumpGameplayCues(const TArray<FString>& Args);

	// 当延迟加载提示时，这将加载无论如何都必须加载的提示
	void LoadAlwaysLoadedCues();

	// 更新单一游戏线索主要资产的捆绑包
	void RefreshGameplayCuePrimaryAsset();

private:
	void OnGameplayTagLoaded(const FGameplayTag& Tag);
	void HandlePostGarbageCollect();
	void ProcessLoadedTags();
	void ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject);
	void OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue);
	void RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject);
	void HandlePostLoadMap(UWorld* NewWorld);
	void UpdateDelayLoadDelegateListeners();
	bool ShouldDelayLoadGameplayCues() const;

	struct FLoadedGameplayTagToProcessData
	{
		FGameplayTag Tag;
		TWeakObjectPtr<UObject> WeakOwner;

		FLoadedGameplayTagToProcessData() { ; }

		FLoadedGameplayTagToProcessData(const FGameplayTag& InTag, const TWeakObjectPtr<UObject>& InWeakOwner) : Tag(InTag), WeakOwner(InWeakOwner) { ; }
	};

	// 由于内容引用而在客户端上预加载的提示
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;

	// 在客户端上预加载并将始终加载的提示（代码引用或显式始终加载）
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagToProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;
	bool bProcessLoadedTagsAfterGC = false;
};

#undef UE_API
