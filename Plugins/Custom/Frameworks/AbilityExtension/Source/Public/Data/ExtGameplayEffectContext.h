// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"
#include "ExtGameplayEffectContext.generated.h"

/** 效果上下文的扩展类型，复制时独立保存命中结果，当前未新增序列化字段。 */
USTRUCT()
struct FExtGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FExtGameplayEffectContext() { ; }

	/** 返回扩展类型信息，供类型检查及序列化使用。 */
	virtual UScriptStruct* GetScriptStruct() const override;

	/** 复制上下文，并深拷贝已有命中结果。 */
	virtual FGameplayEffectContext* Duplicate() const override;

	/** 使用父类逻辑序列化效果上下文。 */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** 校验实际类型并提取扩展上下文；类型不符或句柄为空时返回空指针。 */
	static FExtGameplayEffectContext* ExtractEffectContext(FGameplayEffectContextHandle Handle);

	// 自定义 UPROPERTY() 字段	需要传额外的上下文数据
	// 自定义 Setter/Getter	给外部提供便捷访问
	// 构造函数重载	创建时方便传入初始值
	// UPROPERTY()
	// int32 CartridgeID = -1;
};

template <>
struct TStructOpsTypeTraits<FExtGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FExtGameplayEffectContext>
{
	enum
	{
		/** 使用自定义网络序列化入口。 */
		WithNetSerializer = true,
		/** 允许引擎复制结构体数据。 */
		WithCopy = true
	};
};

// #include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
//使用 Iris (UE5) 网络
// namespace UE::Net
// {
// 	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(
// 		FExtGameplayEffectContext, 
// 		FGameplayEffectContextNetSerializer
// 	);
// }
