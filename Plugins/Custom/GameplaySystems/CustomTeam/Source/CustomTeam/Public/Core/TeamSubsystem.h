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

/*
 * 用于在 Subsystem 中保存并使用的团队信息
 */
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

	// 团队资产改变的委托
	UPROPERTY()
	FOnTeamDisplayAssetChangedDelegate OnTeamDisplayAssetChanged;

public:
	void SetTeamInfo(ATeamInfoBase* Info);
	void RemoveTeamInfo(ATeamInfoBase* Info);
};

// 比较两位演员团队归属的结果
UENUM(BlueprintType)
enum class ETeamComparison : uint8
{
	// 同队伍
	OnSameTeam,

	// 不同队伍
	DifferentTeams,

	// 其中一个团队无效或者Actor无效
	InvalidArgument
};

/**
 * 一个子系统，便于基于团队的角色（例如，棋子或玩家状态）访问团队信息
 */
UCLASS(MinimalAPI)
class UTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	//~End of USubsystem interface

	// 尝试注册新队伍
	UE_API bool RegisterTeamInfo(ATeamInfoBase* TeamInfoBase);
	// 尝试取消注册队伍，如果失败会返回false
	UE_API bool UnregisterTeamInfo(ATeamInfoBase* TeamInfoBase);

	// 用Actor的团队ID变更为新队伍ID
	UE_API bool ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId);

	// 返回该对象所属的队伍，若不属于队伍则返回INDEX_NONE
	// 依次按照 对象自身, Actor发起者, 团队信息Actor, 相关的PS 中查找团队ID
	UE_API int32 FindTeamFromObject(const UObject* TestObject) const;

	// 返回该对象所属的队伍，若不属于队伍则返回INDEX_NONE
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query", meta=(Keywords="Get"))
	UE_API void FindTeamFromActor(const UObject* TestActor, bool& bIsPartOfTeam, int32& TeamId) const;

	// 比较两个参与者的队伍，返回一个值，表示他们是在同一队伍、不同队伍，还是两队都无效
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query", meta=(ExpandEnumAsExecs=ReturnValue))
	UE_API ETeamComparison CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const;

	// 比较两个参与者的队伍，返回一个值，表示他们是在同一队伍、不同队伍，还是两队都无效
	UE_API ETeamComparison CompareTeams(const UObject* A, const UObject* B) const;

	// 如果指定团队存在，则返回为真
	UFUNCTION(BlueprintCallable, Category="Teams|Query")
	UE_API bool DoesTeamExist(int32 TeamId) const;

	// 获取队伍的所有ID
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="Teams|Query")
	UE_API TArray<int32> GetTeamIDs() const;

	// 判定是否允许造成伤害(完全由规则数组驱动, 见 AddDamageRule)
	UE_API bool CanCauseDamage(const UObject* Instigator, const UObject* Target) const;

	// 追加一条伤害规则(服务端权威调用; 规则对象由子系统持有, 防GC)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void AddDamageRule(UTeamDamageRuleBase* Rule);

	// 按规则类添加一条伤害规则(内部自动实例化并持有; 蓝图传入规则类即可, 无需手动创建对象)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void AddDamageRuleFromClass(TSubclassOf<UTeamDamageRuleBase> RuleClass);

	// 移除一条伤害规则
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void RemoveDamageRule(UTeamDamageRuleBase* Rule);

	// 清空所有伤害规则(恢复"全部禁止"的裸状态, 项目自行重建规则集)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Teams|Modify")
	UE_API void ClearDamageRules();

	// 获取当前已注册的所有伤害规则(调试用)
	UFUNCTION(BlueprintCallable, Category="Teams|Modify")
	UE_API TArray<UTeamDamageRuleBase*> GetDamageRules() const;

	// 从指定球队的视角获取该队的展示资产（你还得指定一个查看器，以防游戏模式是“本地玩家总是蓝队”的情况）。
	UFUNCTION(BlueprintCallable, Category="Teams|TeamAsset")
	UE_API UTeamDisplayAssetBase* GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId);

	// 从指定球队的视角获取该队的展示资产（你还得指定一个查看器，以防游戏模式是“本地玩家总是蓝队”的情况）。
	UFUNCTION(BlueprintCallable, Category="Teams|TeamAsset")
	UE_API UTeamDisplayAssetBase* GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent);

	// 当团队显示资产被编辑时调用，会使所有团队颜色观察者更新
	UE_API void NotifyTeamDisplayAssetModified(UTeamDisplayAssetBase* ModifiedAsset);

	// 注册团队ID,并且获取其团队资产改变的委托
	UE_API FOnTeamDisplayAssetChangedDelegate& GetTeamDisplayAssetChangedDelegate(int32 TeamId);

protected:
	UE_API const APlayerState* FindPlayerStateFromActor(const AActor* PossibleTeamActor) const;

private:
	// 全部队伍的队伍信息映射表
	UPROPERTY()
	TMap<int32, FTeamTrackingInfo> TeamMap;

	// 已注册的伤害规则数组(UPROPERTY 持有, 防止被GC; 综合规则: Block 优先于 Allow, 顺序无关)
	UPROPERTY()
	TArray<TObjectPtr<UTeamDamageRuleBase>> DamageRules;

	FDelegateHandle CheatManagerRegistrationHandle;
};
#undef UE_API
