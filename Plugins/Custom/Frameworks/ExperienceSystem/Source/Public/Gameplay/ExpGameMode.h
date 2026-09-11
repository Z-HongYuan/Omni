// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"
#include "Data/ExpDefinition.h"
#include "ExpGameMode.generated.h"

#define UE_API EXPERIENCESYSTEM_API

//登录后事件，当玩家或机器人加入游戏时以及在无缝和非无缝旅行后触发,这在玩家完成初始化后调用
DECLARE_MULTICAST_DELEGATE_TwoParams(FExpGameModePlayerInitialized, AGameModeBase*, AController*);

/*
 * 游戏模式类
 * 控制整个体验系统的启动流程
 */
UCLASS(MinimalAPI)
class AExpGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AExpGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Pawn")
	UE_API const UExpPawnData* GetPawnDataForController(const AController* InController) const;

	// 初始化体验
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	UE_API virtual void InitGameState() override;

	// 处理新玩家加入游戏,本质上将会阻止所有玩家直到Experience加载完成,并且使用重生流程处理玩家,并且在生成时会自动将PawnData注入到PawnExpComponent内
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;

	// 出生点 全部交给 GS 中的生成组件控制,这样处理的话,每次生成都会转发到组件中控制
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 重生流程的控制链条,将会合并PC和AI的生成流程,并且接入到生成组件控制
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	UE_API virtual bool ControllerCanRestart(AController* Controller);
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;

	//重新启动（重生）指定的玩家或机器人下一帧
	//-如果 bForceReset 为真，控制器将重置此帧（放弃当前拥有的典当，如果有的话）
	UFUNCTION(BlueprintCallable, Category = "Experience")
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// 当玩家初始化时触发的委托
	FExpGameModePlayerInitialized OnGameModePlayerInitialized;

protected:
	// 兜底体验：当所有来源（URL / 开发者设置 / 命令行 / 世界设置）都识别不出体验时调用
	// 默认返回 invalid，此时只会打一条错误日志（加载屏会一直挂着）
	// 框架层刻意不硬编码任何资产名，项目层可 override 返回自己的默认体验：
	//   return FPrimaryAssetId(FPrimaryAssetType("ExperienceDefinition"), FName("B_DefaultExperience"));
	UE_API virtual FPrimaryAssetId GetFallbackExperienceId() const;

	// 专用服务器：尝试以服务器身份登录
	// 返回 true 表示本函数已接管后续流程（登录完成后应由项目层调用 HostDedicatedServerMatch 换图），MatchExperience 会直接返回
	// 返回 false 表示当前不是专用服务器场景，继续走兜底体验
	//
	// 默认返回 false。此 GameMode 不实现登录策略，具体实现交由项目层 override。
	// 项目层参考实现（注意类型是 Omni 的 GameUser 插件，不是引擎的 CommonUser）：
	//   if (GetWorld()->GetNetMode() == NM_DedicatedServer && GetWorld()->URL.Map == UGameMapsSettings::GetGameDefaultMap())
	//   {
	//       UGameUserSubsystem* UserSubsystem = GetGameInstance()->GetSubsystem<UGameUserSubsystem>();
	//       UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::OnUserInitializedForDedicatedServer);
	//       if (!UserSubsystem->TryToLoginForOnlinePlay(0))
	//       {
	//           OnUserInitializedForDedicatedServer(nullptr, false, FText(), EGameUserPrivilege::CanPlayOnline, EGameUserOnlineContext::Default);
	//       }
	//       return true;
	//   }
	//   return false;
	// 其中 OnUserInitializedForDedicatedServer 需自行声明为 UFUNCTION 回调，内部调用 HostDedicatedServerMatch()
	UE_API virtual bool TryDedicatedServerLogin();

	// 专用服务器：登录完成后主持一场比赛
	// 通常做法是找到要用的体验 → 构造 UGameUserSession_HostSessionRequest → 调用 UGameUserSessionSubsystem::HostSession 触发换图
	// 默认空实现，由项目层 override
	UE_API virtual void HostDedicatedServerMatch();

private:
	void MatchExperience();
	void MatchExperienceComplete(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource);

	void OnExperienceLoaded(const UExpDefinition* ExperienceDefinition);
	bool IsExperienceLoaded() const;
};
#undef UE_API
