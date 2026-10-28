// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "UObject/PrimaryAssetId.h"
#include "ExpUserFacingDefinition.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UGameUserSession_HostSessionRequest;
class UTexture2D;
class UUserWidget;

/**
 * 面向前端的玩法定义，组合地图、体验和列表展示信息，并生成托管会话请求。
 * 地图与体验使用主资产 ID；调用方将请求交给会话子系统，GameMode 从旅行 URL 读取 Experience。
 */
UCLASS(MinimalAPI, BlueprintType)
class UExpUserFacingDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 本局地图的主资产 ID。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience", meta = (AllowedTypes = "Map"))
	FPrimaryAssetId MapID;

	/** 本局体验的主资产 ID，其名称写入旅行 URL 的 Experience 选项。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience", meta = (AllowedTypes = "ExpDefinition"))
	FPrimaryAssetId ExperienceID;

	/** 追加到旅行 URL 的选项；Experience 同名项由 ExperienceID 覆盖。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	TMap<FString, FString> ExtraArgs;

	/** 托管会话请求中的最大玩家数。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	int32 MaxPlayerCount = 4;

	/** 供前端选择默认玩法的标记，具体选择策略由调用方实现。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	bool bIsDefaultExperience = false;

	/** 前端列表的主标题。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileTitle;

	/** 前端列表的副标题。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileSubTitle;

	/** 前端列表的详细描述。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileDescription;

	/** 前端列表的图标。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TObjectPtr<UTexture2D> TileIcon;

	/** 是否纳入只加载前端可见玩法的结果。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	bool bShowInFrontEnd = true;

	/** 供调用方选择加载界面的软类引用，本类不负责应用或回退策略。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSoftClassPtr<UUserWidget> LoadingScreenWidget;

	/** 地图与体验主资产均已注册且对应包存在时，允许前端提供进入入口。 */
	UFUNCTION(BlueprintPure, Category = "Experience")
	UE_API bool CanHostExperience() const;

	/** 生成托管会话请求，由调用方持有并提交给 HostSession；通过上下文获取会话子系统，缺失时创建临时请求。 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Experience", meta = (WorldContext = "WorldContextObject"))
	UE_API UGameUserSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;

	/** 同步加载并列出玩法定义；bFrontEndOnly 为真时只返回前端可见项，避免在高频路径调用。 */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	static UE_API TArray<UExpUserFacingDefinition*> LoadAllPlaylists(bool bFrontEndOnly = true);
};

#undef UE_API
