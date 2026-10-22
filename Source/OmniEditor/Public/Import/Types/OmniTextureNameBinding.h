// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OmniTextureNameBinding.generated.h"

/** 每个源纹理对应一行，引用该纹理的所有材质共用此命名配置。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniTextureNameBinding
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="原纹理名"))
	FString SourceTexture;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="已识别用途"))
	FString Usage;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="建议名称"))
	FString SuggestedName;
	/** 完整资产名，可带 T_ 前缀；留空时采用建议名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture Naming", meta=(DisplayName="自定义名称（留空自动）"))
	FString CustomName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="最终名称"))
	FString FinalName;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="状态"))
	FString Status;
	/** 独立保存源标识，名称变更不修改 Interchange UID。 */
	UPROPERTY()
	FString SourceUid;
};
