// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

// #include "Data/GameplayTagStack.h"   // 队伍标签栈已注释（具体项目自行定义信息类字段）
#include "GameFramework/Info.h"
#include "TeamInfoBase.generated.h"

#define UE_API CUSTOMTEAM_API

class UTeamSubsystem;

/** 保存仅初始化时复制的队伍 ID，并在生命周期内向队伍子系统注册。 */
UCLASS(MinimalAPI, Abstract)
class ATeamInfoBase : public AInfo
{
	GENERATED_BODY()

public:
	UE_API ATeamInfoBase(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UE_API int32 GetTeamId() const { return TeamId; }
	/** 仅由权威端设置一次有效队伍 ID，并尝试注册到队伍子系统。 */
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
