// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "CustomGamePhaseManager.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

class UCustomGamePhaseAbility;

DECLARE_DYNAMIC_DELEGATE_OneParam(FCustomGamePhaseDynamicDelegate, const UCustomGamePhaseAbility*, Phase);

DECLARE_DELEGATE_OneParam(FCustomGamePhaseDelegate, const UCustomGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FCustomGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);

DECLARE_DELEGATE_OneParam(FCustomGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

// 配对规则
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	// 精确匹配 例如 "A.B" 配对 A.B 不配对 A.B.C)
	ExactMatch,

	// 模糊匹配 例如 "A.B" 配对 A.B 和 A.B.C)
	PartialMatch
};

/**
 * 管理游戏阶段的管理器
 */
UCLASS(MinimalAPI)
class UCustomGamePhaseManager : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UCustomGamePhaseAbility;

public:
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UE_API void StartPhase(TSubclassOf<UCustomGamePhaseAbility> PhaseAbility, const FCustomGamePhaseDelegate& PhaseEndedCallback = FCustomGamePhaseDelegate());

	//TODO 返回一个句柄，以便人们可以删除这些。他们只会增长，直到世界重置。
	//TODO 我们是否应该偶尔清理这些观察者？即使有把手，也不是每个人都会正确地解开它们。
	UE_API void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCustomGamePhaseTagDelegate& WhenPhaseActive);
	UE_API void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCustomGamePhaseTagDelegate& WhenPhaseEnd);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	UE_API bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	UE_API virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UCustomGamePhaseAbility> Phase, const FCustomGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCustomGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCustomGamePhaseTagDynamicDelegate WhenPhaseEnd);

	void OnBeginPhase(const UCustomGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const UCustomGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

private:
	// 能力+阶段Tag+回调的映射
	struct FCustomGamePhaseEntry
	{
		FGameplayTag PhaseTag;
		FCustomGamePhaseDelegate PhaseEndedCallback;
	};

	//当前活跃的阶段
	TMap<FGameplayAbilitySpecHandle, FCustomGamePhaseEntry> ActivePhaseMap;

	// 注册的监听器
	struct FPhaseObserver
	{
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;
		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FCustomGamePhaseTagDelegate PhaseCallback;
	};

	//监听器
	TArray<FPhaseObserver> PhaseStartObservers;
	TArray<FPhaseObserver> PhaseEndObservers;
};

#undef UE_API
