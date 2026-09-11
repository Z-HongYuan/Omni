// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"
#include "TemplateGameplayEffectContext.generated.h"

/*
 * 最基础的 GameplayEffectContext 继承拓展方法
 */
USTRUCT()
struct FTemplateGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FTemplateGameplayEffectContext() { ; }

	// 必须！告诉引擎"我是自定义类型"
	virtual UScriptStruct* GetScriptStruct() const override;

	// 必须！引擎内部复制 EffectContext 时调用
	virtual FGameplayEffectContext* Duplicate() const override;

	// 必须！如果有网络需求
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	// 必须！静态工厂函数,安全提取
	static FTemplateGameplayEffectContext* ExtractEffectContext(FGameplayEffectContextHandle Handle);

	// 自定义 UPROPERTY() 字段	需要传额外的上下文数据
	// 自定义 Setter/Getter	给外部提供便捷访问
	// 构造函数重载	创建时方便传入初始值
	// UPROPERTY()
	// int32 CartridgeID = -1;
};

template <>
struct TStructOpsTypeTraits<FTemplateGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FTemplateGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true, // 如果需要网络同步
		WithCopy = true // 允许引擎复制
	};
};

// #include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
//使用 Iris (UE5) 网络
// namespace UE::Net
// {
// 	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(
// 		FTemplateGameplayEffectContext, 
// 		FGameplayEffectContextNetSerializer
// 	);
// }
