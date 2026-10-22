// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
#include "Gameplay/ExpAIController.h"
#include "ExpBotCreationComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpDefinition;

/** 体验就绪后由权威端创建 Bot，派生类可定制生成和移除流程。 */
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
	/** 每次 ServerCreateBots 调用创建的 Bot 数量，可由 URL 的 ?NumBots=N 覆盖。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 NumBotsToCreate = 5;

	/** 生成 Bot 时使用的控制器类。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<AExpAIController> BotControllerClass;

	/** 随机名称池；用完后使用带随机编号的 Bot 名称。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TArray<FString> RandomBotNames;

	/** 本组件创建的 Bot 控制器列表，供移除操作使用。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AExpAIController>> SpawnedBotList;

	/** 在权威端创建并初始化一个 Bot，派生类可扩展初始化。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API virtual void SpawnOneBot();

	/** 随机移除一个 Bot，直接销毁 Pawn 和控制器；需死亡表现时由派生类扩展。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API virtual void RemoveOneBot();

	/** 按有效数量新增 Bot，不扣除已有数量；蓝图可覆盖流程，C++ 调用方需保证服务器权限。 */
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = Gameplay)
	UE_API void ServerCreateBots();

	/** 控制器创建并尝试重生后通知蓝图；重生失败时控制器可能尚无 Pawn。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Gameplay)
	UE_API void K2_OnBotSpawned(AExpAIController* BotController);

	/** 从剩余名称池选择名字，耗尽后生成随机名称；允许派生类重写。 */
	UE_API virtual FString CreateBotName(int32 PlayerIndex);

	/** 返回考虑 URL 覆盖后的创建数量。 */
	UE_API virtual int32 GetEffectiveBotCount() const;

private:
	/** 体验就绪后，仅在权威端触发 Bot 创建。 */
	UE_API void OnExperienceLoaded(const UExpDefinition* Experience);

	/** 本轮创建中尚未使用的随机名称。 */
	TArray<FString> RemainingBotNames;
};

#undef UE_API
