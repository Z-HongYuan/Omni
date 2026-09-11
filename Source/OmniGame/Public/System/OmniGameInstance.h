// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtGameInstance.h"
#include "OmniGameInstance.generated.h"

#define UE_API OMNIGAME_API

/**
 * 跨地图存在的项目游戏实例。
 * 通用用户与会话初始化由插件父类负责，项目启动和关闭逻辑在此扩展。
 * B_OmniGameInstance 继承本类，作为项目的蓝图配置入口。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 主玩家控制器的强类型访问，以及登录成功后加载本地玩家共享设置。
 * - 网络加密演示：Token/Ack、旅行 URL 参数及调试密钥/DTLS；不直接作为正式加密方案。
 * - InitState 状态链注册已由 UExpSubsystem 提供，本类无需重复迁入。
 * - 加入会话检查沿用插件父类；原版该重写也仅调用父类，无额外限制策略。
 */
UCLASS(MinimalAPI, Blueprintable, Config = Game)
class UOmniGameInstance : public UExtGameInstance
{
	GENERATED_BODY()

public:
	UE_API UOmniGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void Init() override;
	UE_API virtual void Shutdown() override;
};

#undef UE_API
