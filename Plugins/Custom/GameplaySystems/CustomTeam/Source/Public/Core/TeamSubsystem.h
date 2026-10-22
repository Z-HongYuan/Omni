// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "TeamSubsystem.generated.h"

#define UE_API CUSTOMTEAM_API

class APlayerState;
class ATeamInfoBase;
class UTeamDisplayAssetBase;
class ATeamPrivateInfo;
class ATeamPublicInfo;
class UTeamDamageRuleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamDisplayAssetChangedDelegate, const UTeamDisplayAssetBase*, DisplayAsset);

/** 子系统保存的队伍信息 Actor、展示资产及资产变化委托。 */
USTRUCT()
struct FTeamTrackingInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ATeamPublicInfo> PublicInfo = nullptr;

	UPROPERTY()
	TObjectPtr<ATeamPrivateInfo> PrivateInfo = nullptr;

	UPROPERTY()
	TObjectPtr<UTeamDisplayAssetBase> DisplayAsset = nullptr;

	/** 队伍展示资产变化时触发的委托。 */
	UPROPERTY()
	FOnTeamDisplayAssetChangedDelegate OnTeamDisplayAssetChanged;

public:
	void SetTeamInfo(ATeamInfoBase* Info);
	void RemoveTeamInfo(ATeamInfoBase* Info);
};

/** 两个 Actor 的队伍归属比较结果。 */
UENUM(BlueprintType)
enum class ETeamComparison : uint8
{
	/** 双方属于同一队伍。 */
	OnSameTeam,

	/** 双方属于不同队伍。 */
	DifferentTeams,

	/** 任一 Actor 无效或无法取得队伍归属。 */
	InvalidArgument
};

/** 管理世界内的队伍信息、展示资产和伤害规则，并解析 Actor 的队伍归属。 */
UCLASS(MinimalAPI)
class UTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	//~End of USubsystem interface

	/** 注册队伍信息 Actor，并更新对应展示资产。 */
	UE_API bool RegisterTeamInfo(ATeamInfoBase* TeamInfoBase);
	/** 取消队伍信息 Actor 的注册；无有效队伍记录时返回 false。 */
	UE_API bool UnregisterTeamInfo(ATeamInfoBase* TeamInfoBase);

	/** 通过 Actor 实现的队伍接口设置新 ID；未实现接口时返回 false。 */
	UE_API bool ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId);

	/**
	 * 依次查询对象接口、Actor 的 Instigator 接口、队伍信息 Actor 和关联 PlayerState。
	 * 没有匹配的归属来源时返回 INDEX_NONE。
	 */
	UE_API int32 FindTeamFromObject(const UObject* TestObject) const;

	/** 输出对象队伍 ID 及是否有队伍归属；未找到时 ID 为 INDEX_NONE。 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query", meta=(Keywords="Get"))
	UE_API void FindTeamFromActor(const UObject* TestActor, bool& bIsPartOfTeam, int32& TeamId) const;

	/** 比较两个 Actor 的队伍；任一对象或队伍无效时返回 InvalidArgument。 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query", meta=(ExpandEnumAsExecs=ReturnValue))
	UE_API ETeamComparison CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const;

	/** 比较两个 Actor 的队伍；任一对象或队伍无效时返回 InvalidArgument。 */
	UE_API ETeamComparison CompareTeams(const UObject* A, const UObject* B) const;

	/** 返回队伍映射中是否存在指定 ID 的记录。 */
	UFUNCTION(BlueprintCallable, Category="Teams|Query")
	UE_API bool DoesTeamExist(int32 TeamId) const;

	/** 返回队伍映射中的全部 ID，按升序排列。 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query")
	UE_API TArray<int32> GetTeamIDs() const;

	/** 综合已启用的伤害规则与默认策略；任一 Block 都会否决伤害。 */
	UE_API bool CanCauseDamage(const UObject* Instigator, const UObject* Target) const;

	/** 添加并持有伤害规则，避免规则被 GC；蓝图仅允许权威端调用。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void AddDamageRule(UTeamDamageRuleBase* Rule);

	/** 为非抽象规则类创建实例，以子系统为 Outer 并持有该规则。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void AddDamageRuleFromClass(TSubclassOf<UTeamDamageRuleBase> RuleClass);

	/** 移除指定伤害规则并释放子系统对它的引用。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void RemoveDamageRule(UTeamDamageRuleBase* Rule);

	/** 清空伤害规则；后续伤害判定使用 bAllowDefaultDamage 配置。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void ClearDamageRules();

	/** 返回当前注册的伤害规则，供调试查询。 */
	UFUNCTION(BlueprintCallable, Category="Teams|Modify")
	UE_API TArray<UTeamDamageRuleBase*> GetDamageRules() const;

	/** 返回队伍展示资产；当前未按观察者队伍切换展示。 */
	UFUNCTION(BlueprintCallable, Category="Teams|TeamAsset")
	UE_API UTeamDisplayAssetBase* GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId);

	/** 返回队伍展示资产；当前未按观察者队伍切换展示。 */
	UFUNCTION(BlueprintCallable, Category="Teams|TeamAsset")
	UE_API UTeamDisplayAssetBase* GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent);

	/** 展示资产编辑后，向所有队伍的展示资产观察者广播更新。 */
	UE_API void NotifyTeamDisplayAssetModified(UTeamDisplayAssetBase* ModifiedAsset);

	/** 获取指定队伍的资产变化委托；不存在时先创建跟踪记录。 */
	UE_API FOnTeamDisplayAssetChangedDelegate& GetTeamDisplayAssetChangedDelegate(int32 TeamId);

protected:
	UE_API const APlayerState* FindPlayerStateFromActor(const AActor* PossibleTeamActor) const;

private:
	/** 按队伍 ID 保存的跟踪记录。 */
	UPROPERTY()
	TMap<int32, FTeamTrackingInfo> TeamMap;

	/** 持有已注册伤害规则，防止 GC；判定时 Block 优先于 Allow。 */
	UPROPERTY()
	TArray<TObjectPtr<UTeamDamageRuleBase>> DamageRules;

	FDelegateHandle CheatManagerRegistrationHandle;
};
#undef UE_API
