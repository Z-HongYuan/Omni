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
 * 项目角色基类，保留胶囊和主 Mesh，由项目移动组件提供地面信息。
 * 初始化组件通过 PawnExtension 绑定 PlayerState 的 ASC，并连接生命组件。
 * 退场前先注销当前 Avatar，再解除控制，避免遗漏 ASC 注销通知；重生时机由玩法决定。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UE_API AOmniCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 获取当前绑定的扩展 ASC；未绑定时返回空指针。 */
	UFUNCTION(BlueprintPure, Category = "Omni|AbilitySystem")
	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 读取当前 ASC 的标签；未绑定 ASC 时清空输出。 */
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

	/** 仅服务器可主动退场：触发蓝图 OnReset 并清理 Pawn，不产生伤害或请求重生。 */
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

	/** 死亡完成后的下一帧通知蓝图，随后清理角色。 */
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
