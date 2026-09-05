// Copyright © 2026 张鸿源. All Rights Reserved.


#include "GamPhases/CustomGamePhaseManager.h"

#include "LogCustomAbilitySystem.h"
#include "LogCustomGamePhase.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GamPhases/CustomGamePhaseAbility.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomGamePhaseManager)

bool UCustomGamePhaseManager::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		// 判断多人情况下的创建
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		return true;
	}
	// return World->GetAuthGameMode() != nullptr; 创建时是否需要判断多人的情况
	return false;
}

void UCustomGamePhaseManager::StartPhase(TSubclassOf<UCustomGamePhaseAbility> PhaseAbility, const FCustomGamePhaseDelegate& PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UCustomAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UCustomAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);

		if (FoundSpec && FoundSpec->IsActive())
		{
			FCustomGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UCustomGamePhaseManager::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCustomGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UCustomGamePhaseManager::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCustomGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

bool UCustomGamePhaseManager::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FCustomGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

bool UCustomGamePhaseManager::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UCustomGamePhaseManager::K2_StartPhase(TSubclassOf<UCustomGamePhaseAbility> Phase, const FCustomGamePhaseDynamicDelegate& PhaseEnded)
{
	const FCustomGamePhaseDelegate EndedDelegate = FCustomGamePhaseDelegate::CreateWeakLambda(const_cast<UObject*>(PhaseEnded.GetUObject()), [PhaseEnded](const UCustomGamePhaseAbility* PhaseAbility)
	{
		PhaseEnded.ExecuteIfBound(PhaseAbility);
	});

	StartPhase(Phase, EndedDelegate);
}

void UCustomGamePhaseManager::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCustomGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FCustomGamePhaseTagDelegate ActiveDelegate = FCustomGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag)
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UCustomGamePhaseManager::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCustomGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FCustomGamePhaseTagDelegate EndedDelegate = FCustomGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag)
	{
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UCustomGamePhaseManager::OnBeginPhase(const UCustomGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogCustomGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UCustomAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UCustomAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const UCustomGamePhaseAbility* ActivePhaseAbility = CastChecked<UCustomGamePhaseAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

			// 因此，如果当前活动阶段与传入的阶段标签匹配，我们就允许它。
			// 即多个游戏玩法能力都可以与同一阶段标签相关联。
			// 例如，
			//你可以处于Game.Playing阶段，然后开始一个子阶段，比如Game.Playing.SuddenDeath
			//游戏进行阶段仍将处于活跃状态，如果有人推搡另一个人，比如
			// Game.Playing.ActualSuddenDeath，它将结束 Game.Playing.SuddenDeath 阶段，但 Game.Playing 会
			//继续。同样，如果我们激活了 Game.GameOver，所有的 Game.Playing* 阶段都会结束。
			if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
			{
				UE_LOG(LogCustomAbilitySystem, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UCustomGameplayAbility* LyraAbility, FGameplayAbilitySpecHandle Handle)
				{
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FCustomGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// 通知匹配的观察者该阶段已开始。
		for (const FPhaseObserver& Observer : PhaseStartObservers)
		{
			if (Observer.IsMatch(IncomingPhaseTag))
			{
				Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

void UCustomGamePhaseManager::OnEndPhase(const UCustomGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogCustomAbilitySystem, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FCustomGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// 通知匹配的观察者该阶段已结束。
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool UCustomGamePhaseManager::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch (MatchType)
	{
	case EPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case EPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}
