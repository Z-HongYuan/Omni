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

	FMessageRouterListenerHandle() { ; }

	UE_API void Unregister();

	bool IsValid() const { return ID != 0; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMessageRouterManager> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	// FDelegateHandle StateClearedHandle; 没有使用

	friend UMessageRouterManager;

	FMessageRouterListenerHandle(UMessageRouterManager* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID) { ; }
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

	// 围绕此问题的一些潜在问题添加了一些日志记录和额外变量
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

/**
 * 消息路由
 * 使用 Tag 和 Struct 共同构建的消息总线系统
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
	 * @return 如果提供的世界中存在有效的 GameplayMessageRouter 子系统则返回 true
	 */
	static UE_API bool HasInstance(const UObject* WorldContextObject);

	//~USubsystem 接口
	UE_API virtual void Deinitialize() override;
	//~USubsystem 接口结束

	/**
	 * Broadcast a message on the specified channel
	 *
	 * @param Channel			The message channel to broadcast on
	 * @param Message			The message to send (must be the same type of UScriptStruct expected by the listeners for this channel, otherwise an error will be logged)
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
		int32 HandleID = 0;
	};

	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};

#undef UE_API
