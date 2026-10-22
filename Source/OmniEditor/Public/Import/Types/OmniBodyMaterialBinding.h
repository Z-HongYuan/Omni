// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Import/Types/OmniFaceTextureBinding.h"
#include "OmniBodyMaterialBinding.generated.h"

/** 每个 Body 分区独立选择纹理，避免不同服装图集互相覆盖。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniBodyMaterialBinding
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body", meta=(DisplayName="配置此材质"))
	bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body", meta=(DisplayName="源 Body 材质", GetOptions="GetFaceSourceMaterialOptions", EditCondition="bEnabled"))
	FString SourceMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Body", meta=(DisplayName="纹理参数", TitleProperty="Parameter", NoElementDuplicate, EditCondition="bEnabled"))
	TArray<FOmniFaceTextureBinding> Textures;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Body", meta=(DisplayName="匹配状态"))
	FString Status;
	UPROPERTY(Transient)
	FString ResolvedMaterialUid;
};
