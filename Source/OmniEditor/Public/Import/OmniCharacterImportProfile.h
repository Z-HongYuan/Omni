// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Engine/DataAsset.h"
#include "OmniCharacterImportProfile.generated.h"

class UMaterialInterface;
class UTexture;
class UIKRigDefinition;

UENUM(BlueprintType)
enum class EOmniImportTextureUsage : uint8
{
	Preserve,
	Color,
	Mask,
	Normal
};

/** 纹理名称只匹配本次 FBX，不按文件名跨角色搜索资产。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniImportTextureBinding
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	FName Parameter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	FString TextureNamePattern;
	/** 可选的已导入纹理引用，可用于共用 Ramp 或 SDF。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	TSoftObjectPtr<UTexture> ExistingTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	EOmniImportTextureUsage Usage = EOmniImportTextureUsage::Preserve;
};

/** 按数组顺序匹配，采用首个命中的材质规则。 */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniImportMaterialRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
	FString MaterialNamePattern;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
	TSoftObjectPtr<UMaterialInterface> ParentMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
	TArray<FOmniImportTextureBinding> Textures;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
	TMap<FName, float> ScalarParameters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
	TMap<FName, bool> StaticSwitchParameters;
};

USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniImportRetargetChain
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Retarget")
	FName ChainName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Retarget")
	FName StartBone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Retarget")
	FName EndBone;
};

/** 可共用的导入规则；角色专属配置应放在独立 Profile 资产中。 */
UCLASS(BlueprintType)
class OMNIEDITOR_API UOmniCharacterImportProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 是否在所选导入目录下追加角色名称作为根目录。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	bool bCreateCharacterFolder = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString MeshFolder;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString TextureFolder = TEXT("Textures");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString MaterialFolder = TEXT("Materials");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString AnimationFolder = TEXT("Animations");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString RigFolder = TEXT("Rig");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString PhysicsFolder;
	/** 是否应用通用材质规则；关键词材质模式优先使用对话框配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	bool bConfigureMaterials = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials", meta=(EditCondition="bConfigureMaterials"))
	TArray<FOmniImportMaterialRule> MaterialRules;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	bool bCreateIKRig = true;
	/** 在网格体旁创建并绑定姿势直通 ABP，保留已有后处理动画绑定。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bCreatePostProcessAnimBlueprint = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	FName RetargetRoot = TEXT("pelvis");
	/** 优先使用显式骨骼链；列表为空时尝试 UE 的双足骨骼自动识别。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	TArray<FOmniImportRetargetChain> RetargetChains;
	/** 为具有有效四肢链的新 Rig 配置全身 IK 求解器，保留已有 Rig。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig", meta=(EditCondition="bCreateIKRig"))
	bool bTryAutoFBIK = true;
	/** 只有指定源 Rig 时才生成重定向器。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	TSoftObjectPtr<UIKRigDefinition> SourceIKRig;
	/** 用于 IKR_<Source>To<Character> 的源简称；留空时采用源 Rig 名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	FString SourceRigLabel;
};
