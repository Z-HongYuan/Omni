// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "ExpWorldSettings.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpDefinition;
/** 提供地图默认体验和体验相关编辑器检查的世界设置。 */
UCLASS(MinimalAPI)
class AExpWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UE_API AExpWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	UE_API virtual void CheckForErrors() override;
#endif

#if WITH_EDITORONLY_DATA
	/** 供编辑器启动流程读取的独立模式标记；本类本身不修改 PIE 网络模式。 */
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif

	/** 返回地图默认体验的主资产 ID；未注册为主资产时返回无效 ID 并记录错误。 */
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	/** URL、PIE 开发者设置和命令行均未指定体验时使用的默认数据资产。 */
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftObjectPtr<UExpDefinition> DefaultGameplayExperience;
};
#undef UE_API
