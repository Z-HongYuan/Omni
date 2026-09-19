// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OmniTextureNameBinding.generated.h"

/** One row per source texture, shared across all materials that reference it. */
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
	/** Full asset name, with an optional T_ prefix. Empty accepts the suggestion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture Naming", meta=(DisplayName="自定义名称（留空自动）"))
	FString CustomName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="最终名称"))
	FString FinalName;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="状态"))
	FString Status;
	/** Persist source identity separately from names; never change Interchange UIDs. */
	UPROPERTY()
	FString SourceUid;
};
