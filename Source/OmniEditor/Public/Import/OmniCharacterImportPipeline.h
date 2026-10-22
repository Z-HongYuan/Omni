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

/** 放在 OmniMeshes 管线栈的默认 FBX 资产管线之后执行。 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class OMNIEDITOR_API UOmniCharacterImportPipeline : public UInterchangePipelineBase
{
	GENERATED_BODY()

public:
	UOmniCharacterImportPipeline();

	/** 关闭后使用旧版 Face、Body 分开配置的导入模式。 */
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

	/** 只配置选中的 Face 材质，其余材质按各自的 Body 配置或通用规则处理。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material", meta=(DisplayName="配置 Face 材质", EditCondition="!bUseKeywordMaterials", EditConditionHides))
	bool bConfigureFaceMaterial = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material", meta=(DisplayName="Face 母材质"))
	TSoftObjectPtr<UMaterialInterface> FaceParentMaterial;
	/** 留空时自动识别唯一 Face 材质，排除 Outline、Eye、Brow 和 Mouth 材质。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Face Material",
		meta=(DisplayName="源 Face 材质（留空自动识别）", GetOptions="GetFaceSourceMaterialOptions", EditCondition="!bUseKeywordMaterials && bConfigureFaceMaterial", EditConditionHides))
	FString FaceSourceMaterial;
	/** 随当前资产的导入数据保存；新角色的导入对话框会清空这些选择。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Face Material",
		meta=(DisplayName="Face 纹理参数", TitleProperty="Parameter", NoElementDuplicate, EditCondition="!bUseKeywordMaterials && bConfigureFaceMaterial", EditConditionHides))
	TArray<FOmniFaceTextureBinding> FaceTextures;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Face Material", meta=(DisplayName="Face 配置状态"))
	FString FaceStatus;
	/** 重导入默认保留已有 Face 实例；启用后重新应用当前选择。 */
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
	/** 本次 FBX 导入组共用一份复制的 ToonProfile；留空时优先采用所选 Body 母材质的 Profile。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material Mapping", meta=(DisplayName="资产 TP 模板（同一 FBX 共用）"))
	TSoftObjectPtr<UToonProfile> BodyToonProfileTemplate;
	/** 初次根据源 Body 材质填充分区，排除描边材质；每个分区独立选择纹理。 */
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
	bool bNormalizeTextureNames = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, EditFixedSize, Category="Texture Naming", meta=(DisplayName="纹理名称预览", TitleProperty="SourceTexture", NoElementDuplicate, EditCondition="bNormalizeTextureNames"))
	TArray<FOmniTextureNameBinding> TextureNames;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category="Texture Naming", meta=(DisplayName="纹理命名状态"))
	FString TextureNamingStatus;

	virtual void AdjustSettingsForContext(const FInterchangePipelineContextParams& ContextParams) override;
	virtual void PreDialogCleanup(FName PipelineStackName) override;
	virtual void FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* Container) override;
	virtual bool IsPropertyChangeNeedRefresh(const FPropertyChangedEvent& Event) const override;

	/** 通过 UObject 配置接口保存 Interchange 设置，同时更新运行中的配置缓存。 */
	UFUNCTION(BlueprintCallable, Category="Omni Character")
	static bool SaveProjectSettings();
	/** 按需在网格体旁创建 ABP_<MeshName>_Post，保留已有图和绑定。 */
	UFUNCTION(BlueprintCallable, Category="Omni Character")
	bool EnsurePostProcessAnimationBlueprint(USkeletalMesh* Mesh);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Omni Character")
	TSoftObjectPtr<UOmniCharacterImportProfile> Profile;
	/** 可选的导入组名称；留空时单网格体采用基础 FBX 管线的资产名，多网格体采用源文件名。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category="Omni Character")
	FString CharacterName;
	/** 重导入默认保留通用材质规则的现有结果，启用后重新应用规则。 */
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
	void ApplyRetainedMaterials(const UInterchangeBaseNodeContainer* Container, UObject* Mesh);

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
	void PrepareSharedToonProfile(const UInterchangeBaseNodeContainer* Container);
	void ApplyAssetToonProfile(const UInterchangeBaseNodeContainer* Container, UMaterialInstanceConstant* Material);
	void ApplyAssetToonProfiles(const UInterchangeBaseNodeContainer* Container, UObject* Mesh, bool bReimport);
	UPROPERTY(Transient)
	TObjectPtr<UObject> ToonProfileReimportAsset;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceConstant>> SharedProfileCandidates;
	UPROPERTY(Transient)
	TObjectPtr<UToonProfile> SharedToonProfileTemplate;
	UPROPERTY(Transient)
	TObjectPtr<UToonProfile> SharedToonProfile;
	FString SharedToonProfilePackage;
	bool bSharedToonProfileResolved = false;
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
