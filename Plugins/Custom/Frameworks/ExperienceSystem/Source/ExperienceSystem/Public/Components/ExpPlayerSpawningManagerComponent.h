// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
#include "ExpPlayerSpawningManagerComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class AExpPlayerStart;

/**
 * 出生点管理组件
 */
UCLASS(MinimalAPI)
class UExpPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UE_API UExpPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	//~UActorComponent interface
	UE_API virtual void InitializeComponent() override;
	//~End of UActorComponent interface

protected:
	// 扩展点①：C++ 自定义选点，返回 nullptr 时会继续走蓝图钩子，仍为 nullptr 则回退到默认随机选点
	// 项目层可在此按队伍、阶段、模式过滤 PlayerStarts
	UE_API virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AExpPlayerStart*>& PlayerStarts);

	// 扩展点②：蓝图自定义选点，返回 None 表示交给默认逻辑处理
	UFUNCTION(BlueprintImplementableEvent, Category = "Experience|Spawning", meta = (DisplayName = "OnChoosePlayerStart"))
	UE_API AActor* K2_OnChoosePlayerStart(AController* Player, const TArray<AActor*>& PlayerStarts);

	// 扩展点③：重生完成后回调，C++ 版本
	UE_API virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
	{
	}

	// 扩展点③：重生完成后回调，蓝图版本
	UFUNCTION(BlueprintImplementableEvent, Category = "Experience|Spawning", meta = (DisplayName = "OnFinishRestartPlayer"))
	UE_API void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

	// 工具函数：优先从 Empty 的点里随机取，其次从 Partial 的点里随机取
	UE_API APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AExpPlayerStart*>& FoundStartPoints) const;

private:
	// 以下三个函数由 AExpGameMode 转发调用，不要在别处直接调用
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API bool ControllerCanRestart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class AExpGameMode;

	// 缓存场景中所有的 AExpPlayerStart
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AExpPlayerStart>> CachedPlayerStarts;

	// 关卡流送进来时补缓存
	UE_API void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	// 运行时动态生成的出生点补缓存
	UE_API void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	// 编辑器下支持「从此处播放」
	UE_API APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};

#undef UE_API
