// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"
#include "Data/ExperienceDefinition.h"
#include "ExperienceGameMode.generated.h"

#define UE_API EXPERIENCESYSTEM_API

//登录后事件，当玩家或机器人加入游戏时以及在无缝和非无缝旅行后触发,这在玩家完成初始化后调用
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameModePlayerInitialized, AGameModeBase*, AController*);

/*
 * 游戏模式类
 * 控制整个体验系统的启动流程
 */
UCLASS(MinimalAPI)
class AExperienceGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AExperienceGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Pawn")
	UE_API const UExperiencePawnData* GetPawnDataForController(const AController* InController) const;

	// 初始化体验
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	UE_API virtual void InitGameState() override;

	// 处理新玩家加入游戏
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;

	// 出生点 全部交给 GS 中的生成组件控制
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 重生流程的控制链条
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	UE_API virtual bool ControllerCanRestart(AController* Controller);
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;

	//重新启动（重生）指定的玩家或机器人下一帧
	//-如果 bForceReset 为真，控制器将重置此帧（放弃当前拥有的典当，如果有的话）
	UFUNCTION(BlueprintCallable)
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// 当玩家初始化时触发的委托
	FOnGameModePlayerInitialized OnGameModePlayerInitialized;

private:
	void MatchExperience();
	void MatchExperienceComplete(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource);

	void OnExperienceLoaded(const UExperienceDefinition* ExperienceDefinition);
	UE_API bool IsExperienceLoaded() const;

	// 服务器,多人的处理
	// UE_API bool TryDedicatedServerLogin();
	// UE_API void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode);
	// UFUNCTION()
	// UE_API void OnUserInitializedForDedicatedServer(const UCustomUserInfo* UserInfo, bool bSuccess, FText Error, EUserPrivilege RequestedPrivilege, EUserOnlineContext OnlineContext);
};
#undef UE_API
