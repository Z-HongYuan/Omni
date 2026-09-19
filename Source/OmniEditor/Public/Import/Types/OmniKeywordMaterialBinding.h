// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/Types/OmniFaceTextureBinding.h"
#include "OmniKeywordMaterialBinding.generated.h"

class UMaterialInterface;

UENUM(BlueprintType)
enum class EOmniMaterialRole : uint8
{
	Auto,
	Face,
	Body,
	Outline
};

/** Aliases are matched against original source labels, before assets are renamed. */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniTextureKeywordRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Textures")
	FName Parameter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Textures")
	TArray<FString> Keywords;
};

/** One instance per source material, even when several instances share a parent. */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniKeywordMaterialBinding
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Material", meta=(DisplayName="源材质"))
	FString SourceMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material", meta=(DisplayName="母材质类型"))
	EOmniMaterialRole Role = EOmniMaterialRole::Auto;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Material", meta=(DisplayName="匹配的母材质"))
	TSoftObjectPtr<UMaterialInterface> Parent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Material", meta=(DisplayName="纹理参数", TitleProperty="Parameter", NoElementDuplicate))
	TArray<FOmniFaceTextureBinding> Textures;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Material", meta=(DisplayName="状态"))
	FString Status;
	UPROPERTY()
	FString SourceUid;
	UPROPERTY()
	FString AssetSuffix;
	UPROPERTY()
	FString TextureGroup;
	UPROPERTY()
	EOmniMaterialRole ResolvedRole = EOmniMaterialRole::Body;
};
