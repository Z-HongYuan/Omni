// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "ExpPlayerState.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpPawnData;
class UExtAbilitySystemComponent;

/**
 * 玩家状态
 * 支持 ASC
 * 支持 PawnData
 */
UCLASS(MinimalAPI)
class AExpPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UE_API AExpPlayerState(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PostInitializeComponents() override;
	UE_API virtual void ClientInitialize(class AController* C) override;

	//~ ASC接口
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ ASC接口

	// 用于在 ModularGameplay 中发布的消息名称
	static UE_API const FName NAME_GiveCustomAbilityReady;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 设置 PawnData 将会在 体验加载完成后调用
	UE_API void SetPawnData(const UExpPawnData* InPawnData);

protected:
	// 使用 体验 来加载 PawnData
	UE_API virtual void OnExperienceLoaded(const class UExpDefinition* CurrentExperience);

	UFUNCTION()
	UE_API virtual void OnRep_PawnData() { ; }

private:
	// 玩家使用的 ASC 组件子对象
	UPROPERTY(VisibleAnywhere, Category = "PlayerState")
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UExpPawnData> PawnData;
};

#undef UE_API
