// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
#include "ContraRoundComponent.generated.h"

UENUM(BlueprintType)
enum class EContraRoundPhase : uint8 { Playing, Won, Lost };

/** 灰盒关卡规则：清空敌人并抵达终点获胜，全部耗尽命数失败；状态只由服务器推进。 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMNICONTRARUNTIME_API UContraRoundComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UContraRoundComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Function) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UFUNCTION(BlueprintPure, Category = "Contra")
	EContraRoundPhase GetPhase() const { return Phase; }

	// 服务端收到任一玩家的重开请求后，仅允许第一次请求执行换图。
	void RestartRun();
	UPROPERTY(EditDefaultsOnly, Category = "Contra")
	float GoalX = 2500.0f;

private:
	UPROPERTY(Replicated)
	EContraRoundPhase Phase = EContraRoundPhase::Playing;
	bool bTravelRequested = false;
};
