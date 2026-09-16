// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/PlayerStateComponent.h"
#include "ContraPlayerLifeComponent.generated.h"

class APlayerState;
class AController;
class UExtDeathComponent;

/** 每玩家命数，挂在 PS 上跨 Pawn 保留；死亡结束后销毁旧 Avatar，再交给现有 GM 重生。 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMNICONTRARUNTIME_API UContraPlayerLifeComponent : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	UContraPlayerLifeComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UFUNCTION(BlueprintPure, Category = "Contra")
	int32 GetLives() const { return Lives; }

	UFUNCTION(BlueprintPure, Category = "Contra")
	bool IsProtected() const;
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Contra")
	void ServerRequestRestart();
	UPROPERTY(EditDefaultsOnly, Category = "Contra", meta = (ClampMin = "1"))
	int32 StartingLives = 3;
	UPROPERTY(EditDefaultsOnly, Category = "Contra", meta = (ClampMin = "0.1"))
	float RespawnDelay = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Contra", meta = (ClampMin = "0"))
	float ProtectionSeconds = 2.0f;

private:
	UFUNCTION()
	void PawnChanged(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	UFUNCTION()
	void DeathStarted(AActor* PawnActor);
	void Respawn();
	UPROPERTY(Replicated)
	int32 Lives = 3;
	UPROPERTY(Replicated)
	float ProtectionEndTime = 0.0f;
	TWeakObjectPtr<UExtDeathComponent> BoundDeath;
	TWeakObjectPtr<AController> RespawnController;
	FTimerHandle RespawnTimer;
};
