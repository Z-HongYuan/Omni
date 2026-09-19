// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "InterchangePipelineBase.h"
#include "Import/Types/OmniFaceTextureBinding.h"
#include "Import/Types/OmniBodyMaterialBinding.h"
#include "Import/Types/OmniTextureNameBinding.h"
#include "Import/Types/OmniKeywordMaterialBinding.h"
#include "OmniCharacterImportPipeline.generated.h"

class UOmniCharacterImportProfile;
class USkeletalMesh;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UInterchangeBaseNode;
class UToonProfile;

/** Place AFTER the default FBX assets pipeline in the OmniMeshes stack. */
UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class OMNIEDITOR_API UOmniCharacterImportPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	UOmniCharacterImportPipeline();
	/** Disable only for legacy per-Face/Body import presets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="Material Mapping", meta=(DisplayName="按关键词配置全部材质"))
	bool bUseKeywordMaterials = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material Mapping", meta=(DisplayName="Outline 母材质", EditCondition="bUseKeywordMaterials"))
	TSoftObjectPtr<UMaterialInterface> OutlineParentMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Material Mapping", meta=(DisplayName="材质与纹理匹配", TitleProperty="SourceMaterial", NoElementDuplicate, EditCondition="bUseKeywordMaterials", EditConditionHides))
	TArray<FOmniKeywordMaterialBinding> MaterialMappings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="Material Mapping", meta=(DisplayName="纹理用途关键词", TitleProperty="Parameter", EditCondition="bUseKeywordMaterials"))
	TArray<FOmniTextureKeywordRule> TextureKeywords;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Material Mapping", meta=(DisplayName="匹配状态"))
	FString MaterialMappingStatus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material Mapping", meta=(DisplayName="重导入时更新材质与纹理", EditCondition="bUseKeywordMaterials"))
	bool bUpdateKeywordMaterialsOnReimport = false;
	/** Only configure the selected Face material. Other materials follow the existing profile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material", meta=(DisplayName="配置 Face 材质", EditCondition="!bUseKeywordMaterials", EditConditionHides))
	bool bConfigureFaceMaterial = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material", meta=(DisplayName="Face 母材质"))
	TSoftObjectPtr<UMaterialInterface> FaceParentMaterial;
	/** Empty automatically detects a single Face material, excluding outline/eye/brow/mouth materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material",
		meta=(DisplayName="源 Face 材质（留空自动识别）", GetOptions="GetFaceSourceMaterialOptions", EditCondition="!bUseKeywordMaterials && bConfigureFaceMaterial", EditConditionHides))
	FString FaceSourceMaterial;
	/** Saved with this asset's import data, but not carried into a different character's import dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Face Material",
		meta=(DisplayName="Face 纹理参数", TitleProperty="Parameter", NoElementDuplicate, EditCondition="!bUseKeywordMaterials && bConfigureFaceMaterial", EditConditionHides))
	TArray<FOmniFaceTextureBinding> FaceTextures;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Face Material", meta=(DisplayName="Face 配置状态"))
	FString FaceStatus;
	/** Reimport normally preserves the existing face instance; enable only to reapply these choices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material", meta=(DisplayName="重导入时更新 Face 材质", EditCondition="!bUseKeywordMaterials && bConfigureFaceMaterial", EditConditionHides))
	bool bUpdateFaceMaterialOnReimport = false;
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Face Material", meta=(DisplayName="刷新 Tex_ 参数"))
	void RefreshFaceTextureParameters();
	UFUNCTION(BlueprintPure, Category="Face Material")
	TArray<FString> GetFaceSourceMaterialOptions() const;
	UFUNCTION(BlueprintPure, Category="Face Material")
	TArray<FString> GetFaceSourceTextureOptions() const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body Material", meta=(DisplayName="配置 Body 材质", EditCondition="!bUseKeywordMaterials", EditConditionHides))
	bool bConfigureBodyMaterial = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body Material", meta=(DisplayName="Body 母材质"))
	TSoftObjectPtr<UMaterialInterface> BodyParentMaterial;
	/** Empty copies the single Toon Profile used by the Body parent. Never edits the shared template. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body Material", meta=(DisplayName="Body Profile 模板（留空使用母材质）"))
	TSoftObjectPtr<UToonProfile> BodyToonProfileTemplate;
	/** Initially populated from Body source names, excluding outlines. Each section owns its texture choices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body Material",
		meta=(DisplayName="Body 材质分区", TitleProperty="SourceMaterial", NoElementDuplicate, EditCondition="!bUseKeywordMaterials && bConfigureBodyMaterial", EditConditionHides))
	TArray<FOmniBodyMaterialBinding> BodyMaterials;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Body Material", meta=(DisplayName="Body 配置状态"))
	FString BodyStatus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body Material", meta=(DisplayName="重导入时更新 Body 材质", EditCondition="!bUseKeywordMaterials && bConfigureBodyMaterial", EditConditionHides))
	bool bUpdateBodyMaterialOnReimport = false;
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Body Material", meta=(DisplayName="刷新 Body Tex_ 参数"))
	void RefreshBodyTextureParameters();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Texture Naming", meta=(DisplayName="自动规范纹理名称"))
	bool bNormalizeTextureNames = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Texture Naming", meta=(DisplayName="纹理名称预览", TitleProperty="SourceTexture", NoElementDuplicate, EditCondition="bNormalizeTextureNames"))
	TArray<FOmniTextureNameBinding> TextureNames;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="纹理命名状态"))
	FString TextureNamingStatus;
	virtual void AdjustSettingsForContext(const FInterchangePipelineContextParams& ContextParams) override;
	virtual void PreDialogCleanup(FName PipelineStackName) override;
	virtual void FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* Container) override;
	virtual bool IsPropertyChangeNeedRefresh(const FPropertyChangedEvent& Event) const override;
	/** Persist Interchange settings through UObject config APIs, including the live cache. */
	UFUNCTION(BlueprintCallable, Category="Omni Character")
	static bool SaveProjectSettings();
	/** Create ABP_<MeshName>_Post beside the mesh if needed. Preserve existing graphs/assignments. */
	UFUNCTION(BlueprintCallable, Category="Omni Character")
	bool EnsurePostProcessAnimationBlueprint(USkeletalMesh* Mesh);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Omni Character")
	TSoftObjectPtr<UOmniCharacterImportProfile> Profile;
	/** Optional override. Normally leave empty to use the base FBX pipeline's Asset Name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="Omni Character")
	FString CharacterName;
	/** Material defaults are preserved on reimport unless explicitly requested. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Omni Character")
	bool bReapplyMaterialRulesOnReimport = false;
	virtual bool IsSettingsAreValid(TOptional<FText>& OutInvalidReason) const override;
	virtual bool CanExecuteOnAnyThread(EInterchangePipelineTask PipelineTask) override { return false; }
	virtual bool IsScripted() override { return false; }

protected:
	virtual void ExecutePipeline(UInterchangeBaseNodeContainer* Container,
	                             const TArray<UInterchangeSourceData*>& Sources, const FString& ContentBasePath) override;
	virtual void ExecutePostImportPipeline(const UInterchangeBaseNodeContainer* Container,
	                                       const FString& NodeKey, UObject* CreatedAsset, bool bIsAReimport) override;

private:
	bool ValidateLegacyMaterialRules(const UOmniCharacterImportProfile* Settings, TOptional<FText>& OutInvalidReason) const;
	void PrepareLegacyMaterials(UInterchangeBaseNodeContainer* Container);
	void ApplyLegacyMaterial(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey, UMaterialInstanceConstant* Material, int32 RuleIndex);
	void ApplyRetainedMaterials(const UInterchangeBaseNodeContainer* Container, USkeletalMesh* Mesh);
	void RefreshMaterialSourceData(UInterchangeBaseNodeContainer* Container);
	void RefreshKeywordMaterials(UInterchangeBaseNodeContainer* Container);
	bool ValidateKeywordMaterials(TOptional<FText>& OutInvalidReason) const;
	bool PrepareKeywordMaterials(UInterchangeBaseNodeContainer* Container);
	void ApplyKeywordMaterial(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey, UMaterialInstanceConstant* Material, bool bReimport);
	void ResolveKeywordTextures(const UInterchangeBaseNodeContainer* Container, const UInterchangeBaseNode* SourceMaterial, FOmniKeywordMaterialBinding& Section, FString& Error) const;
	TSet<FName> TextureParametersFromKeywords(const FString& Label) const;
	static FString SourceTextureGroup(const FString& Label);
	static FString MaterialTextureGroup(const FOmniKeywordMaterialBinding& Section, FName Parameter);
	FString KeywordMaterialError;
	UPROPERTY(Transient)
	TMap<FString, int32> KeywordFactoryRows;
	void RefreshTextureNames(UInterchangeBaseNodeContainer* Container, const FString& Character = FString());
	bool ValidateTextureNames(TOptional<FText>& OutInvalidReason) const;
	bool PrepareTextureNames(UInterchangeBaseNodeContainer* Container);
	FString TextureNamingError;
	void RefreshBodySourceData(UInterchangeBaseNodeContainer* Container);
	bool ValidateBodySettings(TOptional<FText>& OutInvalidReason) const;
	bool PrepareBodyMaterials(UInterchangeBaseNodeContainer* Container);
	void ApplyBodyMaterial(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey, UMaterialInstanceConstant* Material, bool bReimport);
	UToonProfile* ResolveBodyToonProfileTemplate() const;
	void ApplyBodyToonProfile(const UInterchangeBaseNodeContainer* Container, USkeletalMesh* Mesh, bool bReimport);
	UPROPERTY(Transient)
	TMap<FString, int32> BodyFactoryRows;
	FString BodyValidationError;
	void RefreshFaceSourceData(UInterchangeBaseNodeContainer* Container);
	bool ValidateFaceSettings(TOptional<FText>& OutInvalidReason) const;
	bool PrepareFaceMaterial(UInterchangeBaseNodeContainer* Container);
	void ApplyFaceMaterial(UInterchangeBaseNodeContainer const* Container, const FString& NodeKey, UMaterialInstanceConstant* Material, bool bReimport);
	UPROPERTY(Transient)
	TObjectPtr<UInterchangeBaseNodeContainer> FaceSourceContainer;
	UPROPERTY(Transient)
	TSet<FString> FaceFactoryKeys;
	FString FaceSourceUid;
	FString FaceValidationError;
	void Report(const FString& Message, bool bError = false);
	UPROPERTY(Transient)
	TObjectPtr<UOmniCharacterImportProfile> ActiveProfile;
	UPROPERTY(Transient)
	TMap<FString, int32> MaterialRuleIndices;
	UPROPERTY(Transient)
	TArray<FString> Messages;
	FString CharacterRoot;
	FString ResolvedName;
	bool bPrepared = false;
};
