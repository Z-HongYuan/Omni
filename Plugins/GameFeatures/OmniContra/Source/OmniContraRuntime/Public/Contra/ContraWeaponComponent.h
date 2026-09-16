// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/PawnComponent.h"
#include "ContraWeaponComponent.generated.h"

/** 灰盒步枪：能力控制持续开火，本组件在服务端检查射速、生命和遮挡后结算命中。 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMNICONTRARUNTIME_API UContraWeaponComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UContraWeaponComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	bool TryFire();
	float GetFireInterval() const { return FMath::Max(FireInterval, 0.05f); }

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.05"))
	float FireInterval = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "1"))
	float Damage = 25.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "1"))
	float Range = 2200.0f;

private:
	UFUNCTION(NetMulticast, Unreliable)
	void ShowShot(FVector_NetQuantize Start, FVector_NetQuantize End);
	double NextFireTime = 0.0;
};
