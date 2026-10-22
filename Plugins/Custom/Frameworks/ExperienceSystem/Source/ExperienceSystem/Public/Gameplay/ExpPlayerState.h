// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "ExpPlayerState.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpPawnData;
class UExtAbilitySystemComponent;

/** 持有玩家 ASC 与复制的 PawnData，体验就绪后由服务器授予配置中的能力集。 */
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

	/** 通过 ModularGameplay 通知能力集已授予的扩展事件名。 */
	static UE_API const FName NAME_GiveCustomAbilityReady;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** 由权威端设置一次 PawnData、授予能力集并通知就绪；已有配置时拒绝覆盖。 */
	UE_API void SetPawnData(const UExpPawnData* InPawnData);

protected:
	/** 体验就绪后通过 GameMode 取得 PawnData，保留项目自定义选择策略。 */
	UE_API virtual void OnExperienceLoaded(const class UExpDefinition* CurrentExperience);

	UFUNCTION()
	UE_API virtual void OnRep_PawnData() { ; }

private:
	/** 由 PlayerState 创建并持有的玩家 ASC 子对象。 */
	UPROPERTY(VisibleAnywhere, Category = "PlayerState")
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UExpPawnData> PawnData;
};

#undef UE_API
