// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
#include "Gameplay/ExpAIController.h"
#include "ExpBotCreationComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpDefinition;

/**
 * Bot 创建组件（抽象基类）
 */
UCLASS(MinimalAPI, Abstract)
class UExpBotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UE_API UExpBotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void BeginPlay() override;
	//~End of UActorComponent interface

protected:
	// 需要创建的 Bot 数量，URL 里的 ?NumBots=N 可以覆盖它
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 NumBotsToCreate = 5;

	// 使用的 Bot 控制器类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<AExpAIController> BotControllerClass;

	// 随机 Bot 名字池，用完之后会回退到自动生成的名字
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TArray<FString> RandomBotNames;

	// 已经创建出来的 Bot 列表
	UPROPERTY(Transient)
	TArray<TObjectPtr<AExpAIController>> SpawnedBotList;

	// 创建一个 Bot，可被派生类重载以加入自定义初始化
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API virtual void SpawnOneBot();

	// 移除一个 Bot，可被派生类重载（例如改为播放死亡表现后再销毁）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API virtual void RemoveOneBot();

	// 创建到 NumBotsToCreate 个 Bot，蓝图可覆盖整个流程
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API void ServerCreateBots();

	// 每个 Bot 生成并完成重生后的钩子，项目层可在此设置队伍、名字、难度等
	UFUNCTION(BlueprintImplementableEvent, Category = Gameplay)
	UE_API void K2_OnBotSpawned(AExpAIController* BotController);

	// 取一个 Bot 名字，可被派生类重载
	UE_API virtual FString CreateBotName(int32 PlayerIndex);

	// 实际生效的 Bot 数量（考虑了 URL 覆盖）
	UE_API virtual int32 GetEffectiveBotCount() const;

private:
	// 体验加载完成后的回调
	UE_API void OnExperienceLoaded(const UExpDefinition* Experience);

	// 剩余可用名字
	TArray<FString> RemainingBotNames;
};

#undef UE_API
