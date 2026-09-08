// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "OmniCharacterBase.generated.h"

#define UE_API OMNIGAME_API

class AController;
class UAbilitySystemComponent;
class UCustomAbilitySystemComponent;
class UExperiencePawnExtensionComponent;
class UInputComponent;

/**
 * 项目使用的基础角色类
 *
 * 职责：把 Pawn 上的关键时机转发给 UExperiencePawnExtensionComponent
 * PawnExtension 自己感知不到控制器 / PlayerState / 输入的变化，必须由 Pawn 驱动，
 * 否则 Spawned → DataAvailable → DataInitialized → GameplayReady 这条状态链推不动。
 *
 * 组件挂载约定：
 * - PawnExtension 在本类 C++ 构造中创建（内核，所有体验相同）
 * - 输入组件 / 相机组件由体验资产通过 GameFeatureAction_AddComponents 添加，
 *   注意输入组件必须 bServerComponent + bClientComponent 都勾选（服务端也要做 ASC 初始化），
 *   相机组件可以只勾客户端
 * - 新玩法请做成平级组件（实现 IGameFrameworkInitStateInterface 并注册自己的状态特性），
 *   不要往本类里堆逻辑，PawnExtension 会等所有特性都就绪后统一推进
 */
UCLASS(MinimalAPI)
class AOmniCharacterBase : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UE_API AOmniCharacterBase(const FObjectInitializer& ObjectInitializer);

	//~ 以下时机必须转发给 PawnExtension，缺一个状态链就断
	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;
	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 获取 PawnExtension 组件 , ASC 缓存的也在里面
	UFUNCTION(BlueprintPure, Category = "Omni|Character")
	UExperiencePawnExtensionComponent* GetPawnExtensionComponent() const { return PawnExtensionComponent; }

	// ASC 并不在本角色上，而是挂在 PlayerState 上，这里只是转发 PawnExtension 缓存的指针
	UFUNCTION(BlueprintPure, Category = "Omni|Character")
	UE_API UCustomAbilitySystemComponent* GetCustomAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// ASC 就绪 / 注销的钩子，派生类可重载
	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExperiencePawnExtensionComponent> PawnExtensionComponent;
};
#undef UE_API
