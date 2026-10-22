// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Data/CosmeticDataTypes.h"
#include "CosmeticsControllerComponent.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/**
 * 在控制器上保存外观需求，并在服务端切换 Pawn 时移除旧部件、应用新部件。
 * 外观配置随控制器保留，不依赖当前 Pawn 的生命周期。
 * TODO: 确认是否需要接入统一初始化流程。
 */
UCLASS(MinimalAPI, ClassGroup=(Cosmetics), meta=(BlueprintSpawnableComponent))
class UCosmeticsControllerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UCosmeticsControllerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	/** 仅由服务端记录并应用常规外观部件。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void AddCharacterPart(const FCharacterPart& NewPart);

	/** 仅由服务端移除首个与 PartToRemove 匹配的外观部件。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveCharacterPart(const FCharacterPart& PartToRemove);

	/** 仅由服务端移除当前 Pawn 的部件并清空控制器记录。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveAllCharacterParts();

	/** 在启用 CheatManager 的构建中重新应用开发者外观设置。 */
	UE_API void ApplyDeveloperSettings();

protected:
	/** 控制器持有的外观描述、来源及当前 Pawn 上的应用句柄。 */
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	TArray<FCharacterPartControllerEntry> CharacterParts;

private:
	UCosmeticsClientComponent* GetPawnCustomizer() const;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void AddCharacterPartInternal(const FCharacterPart& NewPart, ECharacterPartSource Source);

	// 调试外观管理。
	void AddCheatPart(const FCharacterPart& NewPart, bool bSuppressNaturalParts);
	void ClearCheatParts();
	void SetSuppressionOnNaturalParts(bool bSuppressed);

	friend class UCosmeticCheats;
};

#undef UE_API
