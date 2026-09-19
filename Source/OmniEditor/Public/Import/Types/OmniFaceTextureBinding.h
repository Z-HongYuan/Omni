// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "OmniFaceTextureBinding.generated.h"

class UTexture;

UENUM(BlueprintType)
enum class EOmniFaceTextureMode : uint8
{
	ParentDefault UMETA(DisplayName="沿用母材质默认值"),
	AutoMatch UMETA(DisplayName="自动匹配本次 FBX"),
	SourceTexture UMETA(DisplayName="选择本次 FBX 纹理"),
	ExistingTexture UMETA(DisplayName="选择项目已有纹理")
};

/** Rows are discovered from the parent material. Only global Tex_ texture parameters are exposed. */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniFaceTextureBinding
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Face", meta=(DisplayName="参数"))
	FName Parameter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face", meta=(DisplayName="绑定方式"))
	EOmniFaceTextureMode Mode = EOmniFaceTextureMode::ParentDefault;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face", meta=(DisplayName="FBX 纹理", GetOptions="GetFaceSourceTextureOptions", EditCondition="Mode == EOmniFaceTextureMode::SourceTexture", EditConditionHides))
	FString SourceTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face", meta=(DisplayName="已有纹理", EditCondition="Mode == EOmniFaceTextureMode::ExistingTexture", EditConditionHides))
	TSoftObjectPtr<UTexture> Texture;
	/** Optional wildcard. Empty uses the source material connection, then conservative name matching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="Face", meta=(DisplayName="自动匹配名称规则", EditCondition="Mode == EOmniFaceTextureMode::AutoMatch", EditConditionHides))
	FString MatchPattern;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Face", meta=(DisplayName="母材质默认纹理"))
	TSoftObjectPtr<UTexture> DefaultTexture;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Face", meta=(DisplayName="匹配状态"))
	FString Status;
	UPROPERTY(Transient)
	FString ResolvedTextureUid;
	/** Auto-match found no texture: leave this parameter inherited from the parent. */
	UPROPERTY(Transient)
	bool bUseParentDefault = false;
};
