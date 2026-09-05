// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

// #include "Data/GameplayTagStack.h"   // 队伍标签栈已注释（具体项目自行定义信息类字段）
#include "GameFramework/Info.h"
#include "TeamInfoBase.generated.h"

#define UE_API CUSTOMTEAM_API

class UTeamSubsystem;

/*
 * 队伍信息基类
 * 目前只提供 队伍ID的存储和获取
 * 其他信息类需要根据具体项目具体分析
 */
UCLASS(MinimalAPI, Abstract)
class ATeamInfoBase : public AInfo
{
	GENERATED_BODY()

public:
	UE_API ATeamInfoBase(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UE_API int32 GetTeamId() const { return TeamId; }
	UE_API void SetTeamId(int32 NewTeamId);

protected:
	//~AActor interface
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	UE_API virtual void RegisterWithTeamSubsystem(UTeamSubsystem* Subsystem);
	UE_API void TryRegisterWithTeamSubsystem();

	UFUNCTION()
	UE_API void OnRep_TeamId();

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	int32 TeamId;
};

#undef UE_API
