// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Data/CosmeticDataTypes.h"
#include "CosmeticsControllerComponent.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/*
 * 在控制器中添加的外观控制组件
 * 用于配置当前 Pawn 需要生成的外观
 * 会在 Pawn 销毁生成中 持久保存外观需求
 * 
 * @TODO 需不需要初始化流程
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

	// 仅在 Server 中调用 向角色添加外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void AddCharacterPart(const FCharacterPart& NewPart);

	// 仅在 Server 中调用 通过句柄移除对应的外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveCharacterPart(const FCharacterPart& PartToRemove);

	// 仅在 Server 中调用 移除所有添加的外观
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	UE_API void RemoveAllCharacterParts();

	// 在 PIE 中应用开发者设置
	UE_API void ApplyDeveloperSettings();

protected:
	// Controller 保存的初始外观,将会自动生成并附加
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	TArray<FCharacterPartControllerEntry> CharacterParts;

private:
	UCosmeticsClientComponent* GetPawnCustomizer() const;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void AddCharacterPartInternal(const FCharacterPart& NewPart, ECharacterPartSource Source);

	// 针对于 作弊管理器 的函数
	void AddCheatPart(const FCharacterPart& NewPart, bool bSuppressNaturalParts);
	void ClearCheatParts();
	void SetSuppressionOnNaturalParts(bool bSuppressed);

	friend class UCosmeticCheats;
};

#undef UE_API
