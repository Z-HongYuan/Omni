// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "OmniCameraAssetTools.generated.h"

class UCameraAsset;
class UCameraRigAsset;

/** 编辑器资产装配入口，补足 GameplayCameras 未暴露给 Python 的构建接口。 */
UCLASS()
class OMNIEDITOR_API UOmniCameraAssetTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 覆盖传入资产的图；调用方负责创建、保存资产。
	UFUNCTION(BlueprintCallable, Category = "Omni|Editor")
	static bool ConfigureFixedCamera(UCameraAsset* Camera, UCameraRigAsset* Rig, FVector WorldOffset, FRotator WorldRotation);
};
