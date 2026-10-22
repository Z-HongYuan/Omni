// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExtGamePhaseManager.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtGamePhaseAbility;

DECLARE_DYNAMIC_DELEGATE_OneParam(FExtGamePhaseDynamicDelegate, const UExtGamePhaseAbility*, Phase);

DECLARE_DELEGATE_OneParam(FExtGamePhaseDelegate, const UExtGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FExtGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);

DECLARE_DELEGATE_OneParam(FExtGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

/** 阶段通知的标签匹配规则。 */
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	/** 仅匹配相同标签，例如 A.B 不匹配 A.B.C。 */
	ExactMatch,

	/** 匹配指定标签及其子标签，例如 A.B 匹配 A.B.C。 */
	PartialMatch
};

/** 通过 GameState 上的 ASC 驱动阶段能力，维护活跃阶段并通知监听者。 */
UCLASS(MinimalAPI)
class UExtGamePhaseManager : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UExtGamePhaseAbility;

public:
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UE_API void StartPhase(TSubclassOf<UExtGamePhaseAbility> PhaseAbility, const FExtGamePhaseDelegate& PhaseEndedCallback = FExtGamePhaseDelegate());

	// TODO: 返回可注销的监听句柄，目前监听记录只随世界重置释放。
	// TODO: 考虑定期清理失效监听者，避免遗漏解绑后持续累积。
	/** 注册开始通知；已有自身或子阶段活跃时立即回调，后续通知按 MatchType 匹配。 */
	UE_API void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FExtGamePhaseTagDelegate& WhenPhaseActive);
	/** 注册阶段结束通知，当前未提供单独注销接口。 */
	UE_API void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FExtGamePhaseTagDelegate& WhenPhaseEnd);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	UE_API bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	UE_API virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UExtGamePhaseAbility> Phase, const FExtGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FExtGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FExtGamePhaseTagDynamicDelegate WhenPhaseEnd);

	void OnBeginPhase(const UExtGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const UExtGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

private:
	/** 活跃阶段对应的标签和结束回调。 */
	struct FExtGamePhaseEntry
	{
		FGameplayTag PhaseTag;
		FExtGamePhaseDelegate PhaseEndedCallback;
	};

	/** 以能力句柄索引的活跃阶段记录。 */
	TMap<FGameplayAbilitySpecHandle, FExtGamePhaseEntry> ActivePhaseMap;

	/** 阶段通知的匹配条件及回调。 */
	struct FPhaseObserver
	{
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;
		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FExtGamePhaseTagDelegate PhaseCallback;
	};

	/** 注册的阶段开始监听者，当前保留到世界子系统销毁。 */
	TArray<FPhaseObserver> PhaseStartObservers;
	/** 注册的阶段结束监听者，当前保留到世界子系统销毁。 */
	TArray<FPhaseObserver> PhaseEndObservers;
};

#undef UE_API
