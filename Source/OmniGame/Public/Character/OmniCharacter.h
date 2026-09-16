// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "OmniCharacter.generated.h"

#define UE_API OMNIGAME_API

class UExpPawnExtensionComponent;
class UExtAbilitySystemComponent;
class UExtHealthComponent;
class UExtDeathComponent;
class UOmniPawnInitializationComponent;

/**
 * 项目角色基类，沿用 Character 的胶囊、骨骼网格和移动组件，外观及具体操控后续配置。
 * 通过初始化组件复用 PlayerState 上的 ASC，并向插件转发控制器、玩家状态和输入事件。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 相机和输入管理器由 GF 游戏动作添加，Character 不默认挂载；IMC 由玩法配置。
 * - GameplayTag/GameplayCue 接口与队伍状态；生命和死亡组件已接入 ASC 生命周期，重生规则由玩法决定。
 * - 自定义移动组件、移动状态标签、加速度压缩与共享移动复制。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UE_API AOmniCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 标准接口供 GAS 查询，插件接口供项目访问扩展能力。
	UFUNCTION(BlueprintPure, Category = "Omni|AbilitySystem")
	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;
	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Omni|Health")
	UExtHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Omni|Death")
	UExtDeathComponent* GetDeathComponent() const { return DeathComponent; }

private:
	void OnAbilitySystemInitialized();
	void OnAbilitySystemUninitialized();

	UFUNCTION()
	void OnOutOfHealth(UExtHealthComponent* Component, float OldValue, float NewValue, AActor* InstigatorActor);
	UFUNCTION()
	void OnDeathStarted(AActor* OwningActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Death", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExtDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExtHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExpPawnExtensionComponent> PawnExtensionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOmniPawnInitializationComponent> PawnInitializationComponent;
};

#undef UE_API
