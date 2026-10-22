// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MessageRouterManager.generated.h"

#define UE_API MESSAGEROUTERS_API

class UAsyncAction_ListenForMessage;
class UMessageRouterManager;

/** 消息监听器的注销句柄；可调用 Unregister 或交给路由器注销。 */
USTRUCT(BlueprintType)
struct FMessageRouterListenerHandle
{
public:
	GENERATED_BODY()

	FMessageRouterListenerHandle() = default;

	UE_API void Unregister();

	bool IsValid() const { return ID != 0; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMessageRouterManager> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	friend UMessageRouterManager;

	FMessageRouterListenerHandle(UMessageRouterManager* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID)
	{
	}
};

/** 记录单个监听器的回调、匹配规则和消息结构类型。 */
USTRUCT()
struct FMessageRouterListenerData
{
	GENERATED_BODY()

	/** 同步接收消息的回调，负载指针仅在分发期间有效。 */
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
	EMessageRouterMatchRule MatchType;

	/** 注册时指定的消息类型弱引用；结合 bHadValidType 区分未指定类型与类型失效。 */
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

/**
 * 游戏实例内的同步消息总线，以 GameplayTag 标识通道，以 USTRUCT 定义消息内容。
 * 广播与回调应在游戏线程执行，不提供网络复制或消息缓存。
 * 同一通道的监听顺序不保证；分发使用监听列表副本，回调中可以注册或注销监听。
 */
UCLASS(MinimalAPI)
class UMessageRouterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend UAsyncAction_ListenForMessage;

public:
	/** 获取上下文所属 GameInstance 的消息路由器；要求世界和子系统有效。 */
	static UE_API UMessageRouterManager& Get(const UObject* WorldContextObject);

	/** 返回上下文所属 GameInstance 是否存在消息路由子系统。 */
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	//~USubsystem 接口
	UE_API virtual void Deinitialize() override;
	//~USubsystem 接口结束

	/**
	 * 在 Channel 上同步广播 Message，全部回调返回后结束。
	 * 消息类型必须兼容监听器声明的结构类型，否则记录错误。
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * 注册指定通道的同步消息回调，并返回用于注销的句柄。
	 * 消息类型必须兼容 FMessageStructType；MatchType 决定是否接收子通道消息。
	 */
	template <typename FMessageStructType>
	FMessageRouterListenerHandle RegisterListener(FGameplayTag Channel, TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback, EMessageRouterMatchRule MatchType = EMessageRouterMatchRule::ExactMatch)
	{
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, ThunkCallback, StructType, MatchType);
	}

	/**
	 * 使用对象成员函数监听指定通道，回调前检查对象的弱引用是否有效。
	 * 不延长对象生命周期；返回的句柄用于注销监听。
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FMessageRouterListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
		                                            [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		                                            {
			                                            if (TOwner* StrongObject = WeakObject.Get())
			                                            {
				                                            (StrongObject->*Function)(Channel, Payload);
			                                            }
		                                            });
	}

	/** 按 Params 中的回调和匹配规则注册监听，返回用于注销的句柄。 */
	template <typename FMessageStructType>
	FMessageRouterListenerHandle RegisterListener(FGameplayTag Channel, FMessageRouterListenerParams<FMessageStructType>& Params)
	{
		FMessageRouterListenerHandle Handle;

		// 仅在配置了回调时注册后续消息监听。
		if (Params.OnMessageReceivedCallback)
		{
			auto ThunkCallback = [InnerCallback = Params.OnMessageReceivedCallback](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
			{
				InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
			};

			const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
			Handle = RegisterListenerInternal(Channel, ThunkCallback, StructType, Params.MatchType);
		}

		return Handle;
	}

	/** 注销 Handle 对应的监听；正在分发的监听列表副本仍可能调用它。 */
	UE_API void UnregisterListener(const FMessageRouterListenerHandle& Handle);

protected:
	/** 从蓝图在指定通道同步广播结构体消息；消息类型必须兼容监听器声明。 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=Messaging, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	UE_API void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	DECLARE_FUNCTION(execK2_BroadcastMessage);

private:
	/** 按结构类型分发消息；MessageBytes 由调用方持有，使用期间不得失效。 */
	UE_API void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	/** 保存监听回调与类型信息，并分配不会复用的监听编号。 */
	UE_API FMessageRouterListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EMessageRouterMatchRule MatchType);

	UE_API void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

	/** 保存同一通道上的全部监听器。 */
	struct FChannelListenerList
	{
		TArray<FMessageRouterListenerData> Listeners;
	};

	TMap<FGameplayTag, FChannelListenerList> ListenerMap;

	/** 累计分配的监听编号；通道清空后仍不重置，避免旧句柄注销新监听。 */
	int32 LastListenerID = 0;
};

#undef UE_API
