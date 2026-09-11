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
 * 面向玩家的玩法定义
 *
 * 职责：
 * - 描述一局游戏"玩什么"：一张地图加一个体验，再带上前端列表要用的标题与图标
 * - 玩家在前端点选某个玩法后，由本资产生成托管会话请求，交给会话子系统完成跳转
 *
 * 注意：
 * - 体验用主资产 ID 引用而不是资产指针，这样项目内容引用玩法插件里的体验时不会触发资产引用校验
 * - 本类只负责"怎么开一局"，真正激活体验仍然是 URL 上的 Experience 选项，由体验模式的配对流程解析
 *
 * 依赖：
 * - 地图与体验都要在 Asset Manager 里注册为主资产，否则生成请求时取不到地图路径
 */
UCLASS(MinimalAPI, BlueprintType)
class UExpUserFacingDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 本局要加载的地图
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience", meta = (AllowedTypes = "Map"))
	FPrimaryAssetId MapID;

	// 本局要激活的体验，名字会作为 URL 选项传给服务器
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience", meta = (AllowedTypes = "ExpDefinition"))
	FPrimaryAssetId ExperienceID;

	// 额外追加到 URL 上的选项
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	TMap<FString, FString> ExtraArgs;

	// 本局允许的最大玩家数
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	int32 MaxPlayerCount = 4;

	// 是否为默认玩法，前端没有指定时优先选中它
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Experience")
	bool bIsDefaultExperience = false;

	// 前端列表的主标题
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileTitle;

	// 前端列表的副标题
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileSubTitle;

	// 前端列表的详细描述
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	FText TileDescription;

	// 前端列表的图标
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TObjectPtr<UTexture2D> TileIcon;

	// 是否显示在前端的玩法列表里
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	bool bShowInFrontEnd = true;

	// 进入本玩法时显示的加载屏，留空则使用项目默认加载屏
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSoftClassPtr<UUserWidget> LoadingScreenWidget;

	// 生成本局使用的托管会话请求，调用方拿到后交给会话子系统的 HostSession
	// WorldContextObject 用来取 GameInstance，进而拿到会话子系统
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Experience", meta = (WorldContext = "WorldContextObject"))
	UE_API UGameUserSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;

	// 列出全部玩法定义，bFrontEndOnly 为真时只返回标记为前端可见的
	// 会在调用时同步把主资产加载进来，只建议在进入前端时调用一次
	UFUNCTION(BlueprintCallable, Category = "Experience")
	static UE_API TArray<UExpUserFacingDefinition*> LoadAllPlaylists(bool bFrontEndOnly = true);
};

#undef UE_API
