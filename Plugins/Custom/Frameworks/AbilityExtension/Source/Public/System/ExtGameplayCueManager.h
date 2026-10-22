// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayCueManager.h"
#include "ExtGameplayCueManager.generated.h"

#define UE_API ABILITYEXTENSION_API

/** GameplayCue 库与标签引用的加载策略。 */
enum class ECueEditorLoadMode
{
	/** 预先异步加载运行时 Cue 库。 */
	LoadUpfront,

	/** 仅在游戏中按标签引用预加载。 */
	PreloadAsCuesAreReferenced_GameOnly,

	/** 游戏和编辑器均按标签引用预加载。 */
	PreloadAsCuesAreReferenced
};

/** 禁止同步补载缺失 GameplayCue，支持异步补载及按引用预加载。 */
UCLASS(MinimalAPI)
class UExtGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	UExtGameplayCueManager() { ; }

	static UE_API UExtGameplayCueManager* Get();

	//~UGameplayCueManager interface
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;
	//~End of UGameplayCueManager interface

	static void DumpGameplayCues(const TArray<FString>& Args);

	/** 在延迟加载模式下预加载常驻 Cue；当前显式标签列表尚未填充。 */
	void LoadAlwaysLoadedCues();

	UE_API void RefreshGameplayCuePrimaryAsset();

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

	/** 因内容对象引用而预加载并持有的 Cue 类。 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;

	/** 无所属对象的常驻 Cue 类，不随内容引用清理。 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagToProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;
	bool bProcessLoadedTagsAfterGC = false;
};

#undef UE_API
