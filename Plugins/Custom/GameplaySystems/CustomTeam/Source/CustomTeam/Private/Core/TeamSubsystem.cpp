// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/TeamSubsystem.h"

#include "LogCustomTeam.h"
#include "Core/TeamAgentInterface.h"
#include "Data/TeamCheats.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Infos/TeamPrivateInfo.h"
#include "Infos/TeamPublicInfo.h"
#include "Rules/TeamDamageDefaultRules.h"
#include "Rules/TeamDamageRuleBase.h"
#include "CustomTeamSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamSubsystem)

void FTeamTrackingInfo::SetTeamInfo(ATeamInfoBase* Info)
{
	if (ATeamPublicInfo* NewPublicInfo = Cast<ATeamPublicInfo>(Info))
	{
		ensure((PublicInfo == nullptr) || (PublicInfo == NewPublicInfo));
		PublicInfo = NewPublicInfo;

		UTeamDisplayAssetBase* OldDisplayAsset = DisplayAsset;
		DisplayAsset = NewPublicInfo->GetTeamDisplayAsset();

		if (OldDisplayAsset != DisplayAsset)
		{
			OnTeamDisplayAssetChanged.Broadcast(DisplayAsset);
		}
	}
	else if (ATeamPrivateInfo* NewPrivateInfo = Cast<ATeamPrivateInfo>(Info))
	{
		ensure((PrivateInfo == nullptr) || (PrivateInfo == NewPrivateInfo));
		PrivateInfo = NewPrivateInfo;
	}
	else
	{
		checkf(false, TEXT("Expected a public or private team info but got %s"), *GetPathNameSafe(Info))
	}
}

void FTeamTrackingInfo::RemoveTeamInfo(ATeamInfoBase* Info)
{
	if (PublicInfo == Info)
	{
		PublicInfo = nullptr;
	}
	else if (PrivateInfo == Info)
	{
		PrivateInfo = nullptr;
	}
	else
	{
		ensureMsgf(false, TEXT("Expected a previously registered team info but got %s"), *GetPathNameSafe(Info));
	}
}

void UTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 非侵入式的添加作弊管理器拓展
	auto AddTeamCheats = [](UCheatManager* CheatManager)
	{
		CheatManager->AddCheatManagerExtension(NewObject<UTeamCheats>(CheatManager));
	};
	CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(AddTeamCheats));

	// 按项目全局设置(Project Settings → Custom Team)挂载基础伤害规则
	const UCustomTeamSettings* Settings = GetDefault<UCustomTeamSettings>();

	// 1. 插件内置默认规则(默认开启; 关闭后需自行配置等效规则, 否则默认"全部禁止")
	if (Settings->bUseDefaultRules)
	{
		AddDamageRuleFromClass(UTeamDamageRule_SelfDamage::StaticClass());
		AddDamageRuleFromClass(UTeamDamageRule_TeamRelationship::StaticClass());
	}

	// 2. 项目配置的全局规则(整个项目的基础规则)
	for (const TSoftClassPtr<UTeamDamageRuleBase>& RuleClass : Settings->GlobalRules)
	{
		if (UClass* LoadedRuleClass = RuleClass.LoadSynchronous())
		{
			AddDamageRuleFromClass(LoadedRuleClass);
		}
	}
}

void UTeamSubsystem::Deinitialize()
{
	UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

	Super::Deinitialize();
}

bool UTeamSubsystem::RegisterTeamInfo(ATeamInfoBase* TeamInfoBase)
{
	if (!ensure(TeamInfoBase))
	{
		return false;
	}

	const int32 TeamId = TeamInfoBase->GetTeamId();

	if (ensure(TeamId != INDEX_NONE))
	{
		FTeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);
		Entry.SetTeamInfo(TeamInfoBase);

		return true;
	}

	return false;
}

bool UTeamSubsystem::UnregisterTeamInfo(ATeamInfoBase* TeamInfoBase)
{
	if (!ensure(TeamInfoBase))
	{
		return false;
	}

	const int32 TeamId = TeamInfoBase->GetTeamId();
	if (ensure(TeamId != INDEX_NONE))
	{
		// 如果找不到这个条目，这很可能是前一个世界的遗留演员，忽略它
		if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
		{
			Entry->RemoveTeamInfo(TeamInfoBase);

			return true;
		}
	}

	return false;
}

bool UTeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId)
{
	const FGenericTeamId NewTeamID = IntegerToGenericTeamId(NewTeamId);

	if (ITeamAgentInterface* TeamActor = Cast<ITeamAgentInterface>(ActorToChange))
	{
		TeamActor->SetGenericTeamId(NewTeamID);
		return true;
	}

	return false;
}

int32 UTeamSubsystem::FindTeamFromObject(const UObject* TestObject) const
{
	// 检查是否实现了团队接口
	if (const ITeamAgentInterface* ObjectWithTeamInterface = Cast<ITeamAgentInterface>(TestObject))
	{
		return GenericTeamIdToInteger(ObjectWithTeamInterface->GetGenericTeamId());
	}

	// 如果是Actor的话,看看他的Instigator的团队信息
	if (const AActor* TestActor = Cast<const AActor>(TestObject))
	{
		// 看看发起者是不是团队成员
		if (const ITeamAgentInterface* InstigatorWithTeamInterface = Cast<ITeamAgentInterface>(TestActor->GetInstigator()))
		{
			return GenericTeamIdToInteger(InstigatorWithTeamInterface->GetGenericTeamId());
		}

		// 或者测试的Actor是团队信息,直接获取他的ID
		if (const ATeamInfoBase* TeamInfo = Cast<ATeamInfoBase>(TestActor))
		{
			return TeamInfo->GetTeamId();
		}

		//看看相关的PS上是否有团队ID
		if (const APlayerState* PlayerState = FindPlayerStateFromActor(TestActor))
		{
			if (const ITeamAgentInterface* PSWithTeamInterface = Cast<ITeamAgentInterface>(PlayerState))
			{
				return GenericTeamIdToInteger(PSWithTeamInterface->GetGenericTeamId());
			}
		}
	}

	return INDEX_NONE;
}

void UTeamSubsystem::FindTeamFromActor(const UObject* TestActor, bool& bIsPartOfTeam, int32& TeamId) const
{
	TeamId = FindTeamFromObject(TestActor);
	bIsPartOfTeam = TeamId != INDEX_NONE;
}

ETeamComparison UTeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const
{
	TeamIdA = FindTeamFromObject(Cast<const AActor>(A));
	TeamIdB = FindTeamFromObject(Cast<const AActor>(B));

	// 如果其中一个对象不属于队伍,则返回InvalidArgument
	if ((TeamIdA == INDEX_NONE) || (TeamIdB == INDEX_NONE))
	{
		return ETeamComparison::InvalidArgument;
	}
	return (TeamIdA == TeamIdB) ? ETeamComparison::OnSameTeam : ETeamComparison::DifferentTeams;
}

ETeamComparison UTeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
	int32 TeamIdA;
	int32 TeamIdB;
	return CompareTeams(A, B, TeamIdA, TeamIdB);
}

bool UTeamSubsystem::DoesTeamExist(int32 TeamId) const
{
	return TeamMap.Contains(TeamId);
}

TArray<int32> UTeamSubsystem::GetTeamIDs() const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);
	Result.Sort();
	return Result;
}

bool UTeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target) const
{
	// 构建伤害判定上下文(队伍关系与同一玩家判定由子系统统一计算)
	FTeamDamageRuleContext Context;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.Relationship = CompareTeams(Instigator, Target);
	Context.bSamePlayer = (Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target)));

	const UCustomTeamSettings* Settings = GetDefault<UCustomTeamSettings>();
	bool bHasAllow = Settings->bAllowDefaultDamage;

	for (const TObjectPtr<UTeamDamageRuleBase>& Rule : DamageRules)
	{
		if (!Rule || !Rule->bEnabled) continue;

		const ETeamDamageRuleResult Result = Rule->EvaluateDamage(Context);
		if (Result == ETeamDamageRuleResult::Block)
		{
			UE_LOGFMT(LogCustomTeam, Display, "{Name}Rule blocked damage", Rule->RuleName.ToString());
			return false;
		}

		if (Result == ETeamDamageRuleResult::Allow)
			bHasAllow = true;

		// 其余就是Continue: 弃权, 继续下一条
	}

	return bHasAllow;
}

void UTeamSubsystem::AddDamageRule(UTeamDamageRuleBase* Rule)
{
	if (Rule) DamageRules.AddUnique(Rule);
}

void UTeamSubsystem::AddDamageRuleFromClass(TSubclassOf<UTeamDamageRuleBase> RuleClass)
{
	// 过滤抽象类(基类本身不可实例化)
	if (RuleClass && !RuleClass->HasAnyClassFlags(CLASS_Abstract))
	{
		AddDamageRule(NewObject<UTeamDamageRuleBase>(this, RuleClass));
	}
}

void UTeamSubsystem::RemoveDamageRule(UTeamDamageRuleBase* Rule)
{
	DamageRules.Remove(Rule);
}

void UTeamSubsystem::ClearDamageRules()
{
	DamageRules.Reset();
}

TArray<UTeamDamageRuleBase*> UTeamSubsystem::GetDamageRules() const
{
	TArray<UTeamDamageRuleBase*> Result;
	Result.Reserve(DamageRules.Num());
	for (const TObjectPtr<UTeamDamageRuleBase>& Rule : DamageRules)
	{
		Result.Add(Rule);
	}
	return Result;
}

UTeamDisplayAssetBase* UTeamSubsystem::GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId)
{
	//TODO 当前忽略ViewerTeamId

	if (FTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		return Entry->DisplayAsset;
	}

	return nullptr;
}

UTeamDisplayAssetBase* UTeamSubsystem::GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent)
{
	return GetTeamDisplayAsset(TeamId, FindTeamFromObject(ViewerTeamAgent));
}

void UTeamSubsystem::NotifyTeamDisplayAssetModified(UTeamDisplayAssetBase* ModifiedAsset)
{
	//当显示资源正在编辑时，向所有观察者广播，而不仅仅是编辑后的显示资源
	for (const auto& KVP : TeamMap)
	{
		const int32 TeamId = KVP.Key;
		const FTeamTrackingInfo& TrackingInfo = KVP.Value;

		TrackingInfo.OnTeamDisplayAssetChanged.Broadcast(TrackingInfo.DisplayAsset);
	}
}

FOnTeamDisplayAssetChangedDelegate& UTeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
	return TeamMap.FindOrAdd(TeamId).OnTeamDisplayAssetChanged;
}

const APlayerState* UTeamSubsystem::FindPlayerStateFromActor(const AActor* PossibleTeamActor) const
{
	if (PossibleTeamActor == nullptr) return nullptr;

	//@TODO：考虑使用一个接口，还是让团队参与者向子系统注册并维护一个映射？（或LWC风格）
	// 如果是Pawn,获取他的PS
	if (const APawn* Pawn = Cast<const APawn>(PossibleTeamActor))
		if (APlayerState* PS = Pawn->GetPlayerState())
			return PS;


	// 如果是PC,获取他的PS
	if (const AController* PC = Cast<const AController>(PossibleTeamActor))
		if (PC->PlayerState)
			return PC->PlayerState;

	// 如果是PS,直接返回
	if (const APlayerState* PS = Cast<const APlayerState>(PossibleTeamActor))
		return PS;


	// 这个逻辑是递归的,直到找到一个PS,找到所有的instigator
	// Try the instigator
	// 		if (AActor* Instigator = PossibleTeamActor->GetInstigator())
	// 		{
	// 			if (ensure(Instigator != PossibleTeamActor))
	// 			{
	// 				return FindPlayerStateFromActor(Instigator);
	// 			}
	// 		}
	return nullptr;
}
