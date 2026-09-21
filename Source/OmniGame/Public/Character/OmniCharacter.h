// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "OmniCharacter.generated.h"

#define UE_API OMNIGAME_API

class UExpPawnExtensionComponent;
class UExtAbilitySystemComponent;
class UExtHealthComponent;
class UOmniPawnInitializationComponent;

/**
 * 项目角色基类，保留 Character 的胶囊和主 Mesh，项目移动组件额外提供地面信息。
 * 通过初始化组件复用 PlayerState 上的 ASC，并向插件转发控制器、玩家状态和输入事件。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 相机和输入管理器由 GF 游戏动作添加，Character 不默认挂载；IMC 由玩法配置。
 * - GameplayCue 接口与队伍状态；出生阶段恢复生命，重生时机由玩法决定。
 * - 死亡收尾沿用 Lyra；本项目在解除控制前注销 ASC，避免 PC 提前清空 Avatar 后漏掉注销通知。
 * - 移动状态标签、加速度压缩与共享移动复制。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UE_API AOmniCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 标准接口供 GAS 查询，插件接口供项目访问扩展能力。
	UFUNCTION(BlueprintPure, Category = "Omni|AbilitySystem")
	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 查询当前绑定的 ASC；未绑定时输出为空，匹配返回 false。
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;
	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	// 主动退场：触发继承的蓝图 OnReset，清理旧 Pawn；不产生伤害，也不请求重生。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Omni|Character")
	UE_API virtual void Reset() override;

	UFUNCTION(BlueprintPure, Category = "Omni|Health")
	UExtHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

	UE_API void DestroyDueToDeath();
	UE_API void DisableMovementAndCollision();
	UE_API void UninitAndDestroy();

	// 在下一帧、实际清理角色之前通知蓝图，与 Lyra 一致。
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeathFinished"))
	UE_API void K2_OnDeathFinished();

private:
	void OnAbilitySystemInitialized();
	void OnAbilitySystemUninitialized();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExtHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExpPawnExtensionComponent> PawnExtensionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOmniPawnInitializationComponent> PawnInitializationComponent;
};

#undef UE_API
