// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
#include "ExpPlayerSpawningManagerComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class AExpPlayerStart;

/** 缓存出生点，为 GameMode 提供选点和重生完成钩子。 */
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
	/** C++ 选点钩子，可过滤候选点；返回空指针时依次尝试蓝图钩子和默认随机选点。 */
	UE_API virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AExpPlayerStart*>& PlayerStarts);

	/** 蓝图选点钩子，返回 None 时使用默认随机选点。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Experience|Spawning", meta = (DisplayName = "OnChoosePlayerStart"))
	UE_API AActor* K2_OnChoosePlayerStart(AController* Player, const TArray<AActor*>& PlayerStarts);

	/** GameMode 转发重生完成流程时调用的 C++ 钩子。 */
	UE_API virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
	{
	}

	/** C++ 重生完成钩子执行后通知蓝图。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Experience|Spawning", meta = (DisplayName = "OnFinishRestartPlayer"))
	UE_API void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

	/** 按碰撞占用状态随机选点，优先 Empty、其次 Partial；不检查声明占用标记。 */
	UE_API APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AExpPlayerStart*>& FoundStartPoints) const;

private:
	// 以下重生入口由友元 AExpGameMode 转发调用。
	UE_API AActor* ChoosePlayerStart(AController* Player);
	UE_API bool ControllerCanRestart(AController* Player);
	UE_API void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class AExpGameMode;

	/** 场景出生点的弱引用缓存，选点时移除已失效引用。 */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AExpPlayerStart>> CachedPlayerStarts;

	/** 同一世界流入关卡时，将其中的出生点加入缓存。 */
	UE_API void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	/** 将运行时生成的出生点加入缓存。 */
	UE_API void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	/** 仅为玩家控制器查找编辑器的“从此处播放”出生点。 */
	UE_API APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};

#undef UE_API
