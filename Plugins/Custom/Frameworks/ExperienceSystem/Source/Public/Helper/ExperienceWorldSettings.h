// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "ExperienceWorldSettings.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExperienceDefinition;
/**
 * 世界设置类
 * 附带体验设置
 */
UCLASS(MinimalAPI)
class AExperienceWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UE_API AExperienceWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	UE_API virtual void CheckForErrors() override;
#endif

#if WITH_EDITORONLY_DATA
	//这个地图是UI体验还是其他独立体验的一部分？
	//设置后，当您在编辑器中点击“播放”时，网络模式将强制为独立模式
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif

	//如果服务器打开关卡时没有被UI体验覆盖，则使用的默认体验
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	//如果服务器打开关卡时没有被UI体验覆盖，则使用的默认体验
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<UExperienceDefinition> DefaultGameplayExperience;
};
#undef UE_API
