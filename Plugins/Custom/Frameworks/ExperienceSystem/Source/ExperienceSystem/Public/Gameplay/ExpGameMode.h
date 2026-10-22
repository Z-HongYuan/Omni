// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameMode.h"
#include "Data/ExpDefinition.h"
#include "ExpGameMode.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/** 玩家或 Bot 完成通用初始化后的通知，包含旅行后的初始化。 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FExpGameModePlayerInitialized, AGameModeBase*, AController*);

/** 选择并启动体验，等待体验就绪后生成玩家；向 Pawn 注入配置并协调玩家与 Bot 的重生流程。 */
UCLASS(MinimalAPI)
class AExpGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AExpGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Pawn")
	UE_API const UExpPawnData* GetPawnDataForController(const AController* InController) const;

	/** 初始化游戏并安排下一帧选择体验。 */
	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	UE_API virtual void InitGameState() override;

	/** 体验就绪前暂缓玩家生成；就绪后交给父类流程。 */
	UE_API virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	UE_API virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	UE_API virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	UE_API virtual void GenericPlayerInitialization(AController* NewPlayer) override;

	// 出生点选择入口；有生成管理组件时转发，否则沿用引擎选点。
	UE_API virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	UE_API virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	UE_API virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 玩家与 AI 共用的重生检查和完成入口。
	UE_API virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	UE_API virtual bool ControllerCanRestart(AController* Controller);
	UE_API virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	UE_API virtual void FailedToRestartPlayer(AController* NewPlayer) override;

	/** 下一帧请求玩家或 Bot 重生；bForceReset 为真时先在本帧重置控制器并放弃当前 Pawn。 */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	UE_API void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	/** 玩家或 Bot 完成通用初始化后的通知。 */
	FExpGameModePlayerInitialized OnGameModePlayerInitialized;

protected:
	// 未启用的兜底示例；实际主资产类型和名称需与项目配置一致。
	//   return FPrimaryAssetId(FPrimaryAssetType("ExperienceDefinition"), FName("B_DefaultExperience"));
	/** 返回项目兜底体验；默认无效，未提供有效体验时加载界面不会结束。 */
	UE_API virtual FPrimaryAssetId GetFallbackExperienceId() const;

	// 未启用的 GameUser 登录示例，保留供项目实现扩展点时参考。
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
	// 示例回调需由项目自行声明并实现，登录完成后负责触发建局。
	/** 专用服务器登录扩展点；返回 true 表示接管后续建局流程，默认返回 false。 */
	UE_API virtual bool TryDedicatedServerLogin();

	/** 专用服务器登录后的建局扩展点，默认空实现，由项目层负责构造会话请求和旅行。 */
	UE_API virtual void HostDedicatedServerMatch();

private:
	void MatchExperience();
	void MatchExperienceComplete(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource);

	void OnExperienceLoaded(const UExpDefinition* ExperienceDefinition);
	bool IsExperienceLoaded() const;
};
#undef UE_API
