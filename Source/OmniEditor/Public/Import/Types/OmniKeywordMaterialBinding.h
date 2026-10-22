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

/** 在资产重命名前使用原始源标签匹配关键词。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniTextureKeywordRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Textures")
	FName Parameter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Textures")
	TArray<FString> Keywords;
};

/** 每个源材质生成独立实例，即使多个实例使用同一母材质。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniKeywordMaterialBinding
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Material", meta=(DisplayName="源材质"))
	FString SourceMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material", meta=(DisplayName="母材质类型"))
	EOmniMaterialRole Role = EOmniMaterialRole::Auto;
	/** 可为当前源材质指定母材质；留空时使用所选类型的默认母材质。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material", meta=(DisplayName="自定义母材质（留空按类型）"))
	TSoftObjectPtr<UMaterialInterface> ParentOverride;
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
