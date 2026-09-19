// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Animation/TaggedAnimInstance.h"
#include "OmniAnimInstance.generated.h"

#define UE_API OMNIGAME_API

class UOmniCMC;

/** 主 Mesh 的项目动画基类：复用 Tag 绑定，提供 Property Access 的类型化移动组件入口。 */
UCLASS(MinimalAPI)
class UOmniAnimInstance : public UTaggedAnimInstance
{
	GENERATED_BODY()

public:
	UE_API UOmniAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 只取得组件，不刷新数据。不要标记 BlueprintThreadSafe：由 Property Access 在游戏线程采样。
	UFUNCTION(BlueprintPure, Category = "Omni|Animation", meta=(BlueprintThreadSafe))
	UE_API UOmniCMC* GetOmniCMC() const;
};

#undef UE_API
