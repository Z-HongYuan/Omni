// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MessageRouterManager.generated.h"

#define UE_API MESSAGEROUTERS_API

class UAsyncAction_ListenForMessage;
class UMessageRouterManager;

/**
 * 一个不透明的句柄，可用于移除之前注册的消息监听器
 * @see UMessageRouterManager::RegisterListener 和 UMessageRouterManager::UnregisterListener
 */
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

/** 
 * 单个已注册监听器的条目信息
 */
USTRUCT()
struct FMessageRouterListenerData
{
	GENERATED_BODY()

	// 收到消息时的回调
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
	EMessageRouterMatchRule MatchType;

	// 记录注册时是否指定消息类型，用于区分未指定类型与类型失效。
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
	/**
	 * @return 与指定对象的世界相关联的游戏实例的消息路由器
	 */
	static UE_API UMessageRouterManager& Get(const UObject* WorldContextObject);

	/**
	 * @return 如果提供的世界中存在有效的消息路由子系统则返回 true
	 */
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	//~USubsystem 接口
	UE_API virtual void Deinitialize() override;
	//~USubsystem 接口结束

	/**
	 * 在指定通道上同步广播消息，所有回调结束后才返回。
	 *
	 * @param Channel			要广播的消息通道
	 * @param Message			消息结构体；类型必须兼容监听器声明的类型，否则会记录错误。
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * 注册以接收指定通道上的消息
	 *
	 * @param Channel			要监听的消息通道
	 * @param Callback			当有人广播消息时要调用的函数（必须是此通道广播者提供的相同 UScriptStruct 类型，否则会记录错误）
	 *
	 * @return 一个可用于注销此监听器的句柄（通过调用句柄上的 Unregister() 或调用路由器上的 UnregisterListener）
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
	 * 注册以接收指定通道上的消息并使用指定的成员函数处理
	 * 在执行回调之前执行弱对象有效性检查，以确保注册函数的对象仍然存在
	 *
	 * @param Channel			要监听的消息通道
	 * @param Object			要在其上调用函数的对象实例
	 * @param Function			当有人广播消息时要调用的成员函数（必须是此通道广播者提供的相同 UScriptStruct 类型，否则会记录错误）
	 *
	 * @return 一个可用于注销此监听器的句柄（通过调用句柄上的 Unregister() 或调用路由器上的 UnregisterListener）
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

	/**
	 * 使用额外参数注册以接收指定通道上的消息，以支持高级行为
	 * 此逻辑的状态部分应该分离到单独的系统中
	 *
	 * @param Channel			要监听的消息通道
	 * @param Params			包含高级行为详细信息的结构体
	 *
	 * @return 一个可用于注销此监听器的句柄（通过调用句柄上的 Unregister() 或调用路由器上的 UnregisterListener）
	 */
	template <typename FMessageStructType>
	FMessageRouterListenerHandle RegisterListener(FGameplayTag Channel, FMessageRouterListenerParams<FMessageStructType>& Params)
	{
		FMessageRouterListenerHandle Handle;

		// 注册以接收此通道上的任何未来消息
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

	/**
	 * 移除之前由 RegisterListener 注册的消息监听器
	 *
	 * @param Handle	RegisterListener 返回的句柄
	 */
	UE_API void UnregisterListener(const FMessageRouterListenerHandle& Handle);

protected:
	/**
	 * 在指定通道上广播消息
	 *
	 * @param Channel			要广播的消息通道
	 * @param Message			要发送的消息（必须是此通道监听器期望的相同 UScriptStruct 类型，否则会记录错误）
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=Messaging, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	UE_API void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	DECLARE_FUNCTION(execK2_BroadcastMessage);

private:
	// 广播消息的内部辅助函数
	UE_API void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	// 注册消息监听器的内部辅助函数
	UE_API FMessageRouterListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EMessageRouterMatchRule MatchType);

	UE_API void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

	// 给定通道的所有条目列表
	struct FChannelListenerList
	{
		TArray<FMessageRouterListenerData> Listeners;
	};

	TMap<FGameplayTag, FChannelListenerList> ListenerMap;

	// 通道清空或重新创建时也不重置，避免旧句柄误注销后来注册的监听。
	int32 LastListenerID = 0;
};

#undef UE_API
