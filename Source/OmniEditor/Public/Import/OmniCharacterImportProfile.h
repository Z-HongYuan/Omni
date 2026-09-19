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

/** Texture names refer to the current FBX only; no global, cross-character filename search. */
USTRUCT(BlueprintType)
struct OMNIEDITOR_API FOmniImportTextureBinding
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	FName Parameter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	FString TextureNamePattern;
	/** Optional reference to an already imported texture, including shared ramps and SDFs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	TSoftObjectPtr<UTexture> ExistingTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture")
	EOmniImportTextureUsage Usage = EOmniImportTextureUsage::Preserve;
};

/** Rules are evaluated in array order; the first matching rule wins. */
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

/** Shared import rules. Character-specific overrides belong in separate profile assets. */
UCLASS(BlueprintType)
class OMNIEDITOR_API UOmniCharacterImportProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The chosen import directory is the root; optionally append the character name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	bool bCreateCharacterFolder = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString MeshFolder;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString TextureFolder = TEXT("Textures");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString MaterialFolder = TEXT("Materials");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString RigFolder = TEXT("Rig");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Folders")
	FString PhysicsFolder;
	/** Keep disabled until the project's master-material parameter interface is ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials")
	bool bConfigureMaterials = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Materials", meta=(EditCondition="bConfigureMaterials"))
	TArray<FOmniImportMaterialRule> MaterialRules;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	bool bCreateIKRig = true;
	/** Create and assign a pose-passthrough ABP beside the mesh; never replace an existing assignment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bCreatePostProcessAnimBlueprint = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	FName RetargetRoot = TEXT("pelvis");
	/** Explicit chains take precedence; an empty list requests UE's biped auto-detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	TArray<FOmniImportRetargetChain> RetargetChains;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	bool bTryAutoFBIK = false;
	/** Retargeter generation is enabled only when this source rig is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	TSoftObjectPtr<UIKRigDefinition> SourceIKRig;
	/** Short source label for IKR_<Source>To<Character>. Empty uses the source rig name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rig")
	FString SourceRigLabel;
};
