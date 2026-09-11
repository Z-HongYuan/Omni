// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPtr.h"

#define UE_API ASYNCLOADMIXIN_API

class FAsyncCondition;
class FName;
struct FStreamableHandle;

DECLARE_DELEGATE_OneParam(FStreamableHandleDelegate, TSharedPtr<FStreamableHandle>)

// 待办：考虑引入资源保留策略，预加载的资源会自动保留在内存中直到被取消。
//     但如果你想仅使用 AsyncLoad 函数来预加载单个项目怎么办？我不想
//     为每次调用引入单独的策略，或者引入一整套预加载与异步加载的区别，所以更
//     倾向于有一个保留策略。它应该是一个成员并在继承混入类时实际分配内存，
//     还是应该作为一个模板参数？
//enum class EAsyncMixinRetentionPolicy : uint8
//{
//	Default,
//	KeepResidentUntilComplete,
//	KeepResidentUntilCancel
//};

/**
 * FAsyncLoadMixin 允许更轻松地管理异步加载请求，确保线性请求处理，使编写代码更加简单。使用模式如下：
 *
 * 首先 - 继承自 FAsyncLoadMixin，即使你是 UObject，也可以同时继承自 FAsyncLoadMixin。
 *
 * 然后 - 你可以按以下方式发起异步加载：
 * 
 * CancelAsyncLoading();			// 某些对象会被复用（比如在列表中），因此取消任何待处理的请求很重要，避免它们完成。
 * AsyncLoad(ItemOne, CallbackOne);
 * AsyncLoad(ItemTwo, CallbackTwo);
 * StartAsyncLoading();
 * 
 * 你也可以捕获 this。FAsyncLoadMixin 会随宿主派生对象的销毁清理异步加载状态。
 * 例如：
 * AsyncLoad(SomeSoftObjectPtr, [this, ...]() {
 *    
 * });
 * 
 *
 * 首先取消现有的加载请求，例如控件被复用并需要展示新的对象时。
 * 接下来我们会加载 ItemOne 和 ItemTwo，*然后* 按照你请求异步加载的顺序调用回调函数 - 
 * 即使 ItemOne 或 ItemTwo 在你请求时已经加载完成。
 *
 * 当所有异步加载请求完成时，OnFinishedLoading 将被调用。
 * 
 * 如果你忘记调用 StartAsyncLoading()，我们会在下一帧调用它，但你应该在完成设置后记得调用它，
 * 因为可能所有内容都已经加载完毕，这样可以避免单帧的加载指示器闪烁，这很烦人。
 * 
 * 注意：在匿名函数中捕获 [this] 时，应让加载请求的生命周期与宿主一致。
 * 宿主析构或主动取消加载时，混入类会清理对应的加载状态。
 *
 * 注意：FAsyncLoadMixin 不在宿主中保存加载状态成员。通常，手动管理异步加载的类会保存
 * TSharedPtr<FStreamableHandle> 成员并倾向于持有 SoftObjectPaths 临时状态。
 * FAsyncLoadMixin 在内部使用静态 TMap 完成所有这些操作，以便所有异步请求内存都稀疏地临时存储。
 * 
 * 注意：为了调试和了解正在发生的事情，你应该将 -LogCMD="LogAsyncMixin Verbose" 添加到命令行。
 */
class FAsyncLoadMixin : public FNoncopyable
{
protected:
	UE_API FAsyncLoadMixin();

public:
	UE_API virtual ~FAsyncLoadMixin();

protected:
	/** 当加载开始时调用。 */
	virtual void OnStartedLoading()
	{
	}

	/** 当所有加载完成时调用。 */
	virtual void OnFinishedLoading()
	{
	}

protected:
	/** 异步加载 TSoftClassPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载 TSoftClassPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void(TSubclassOf<T>)>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(),
		          FSimpleDelegate::CreateLambda([SoftClass, UserCallback = MoveTemp(Callback)]() mutable
		          {
			          UserCallback(SoftClass.Get());
		          })
		);
	}

	/** 异步加载 TSoftClassPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), Callback);
	}

	/** 异步加载 TSoftObjectPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载 TSoftObjectPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void(T*)>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(),
		          FSimpleDelegate::CreateLambda([SoftObject, UserCallback = MoveTemp(Callback)]() mutable
		          {
			          UserCallback(SoftObject.Get());
		          })
		);
	}

	/** 异步加载 TSoftObjectPtr<T>，完成时调用回调。 */
	template <typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), Callback);
	}

	/** 异步加载 FSoftObjectPath，完成时调用回调。 */
	UE_API void AsyncLoad(const FSoftObjectPath& SoftObjectPath, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 异步加载 FSoftObjectPath 数组，完成时调用回调。 */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObjectPaths, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载 FSoftObjectPath 数组，完成时调用回调。 */
	UE_API void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 给定主资产数组，加载指定资源包；跳过空对象和无效的主资产 ID。资源包由 LoadBundles 指定。 */
	template <typename T = UPrimaryDataAsset>
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<T*>& Assets, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		TArray<FPrimaryAssetId> PrimaryAssetIds;
		PrimaryAssetIds.Reserve(Assets.Num());
		for (const T* Item : Assets)
		{
			if (Item)
			{
				if (const FPrimaryAssetId AssetId = Item->GetPrimaryAssetId(); AssetId.IsValid())
				{
					PrimaryAssetIds.Add(AssetId);
				}
			}
		}

		AsyncPreloadPrimaryAssetsAndBundles(PrimaryAssetIds, LoadBundles, Callback);
	}

	/** 给定主资产 ID 数组，加载这些资产的属性所引用的所有捆绑包（由 LoadBundles 数组指定）。 */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, TFunction<void()>&& Callback)
	{
		AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 给定主资产 ID 数组，加载这些资产的属性所引用的所有捆绑包（由 LoadBundles 数组指定）。 */
	UE_API void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 添加一个必须为真的未来条件，然后才能继续前进。 */
	UE_API void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * 不加载任何内容，而是将此回调插入到回调序列中，以便当异步加载完成时，
	 * 此事件将在序列中的相同点被调用。如果你不希望某个步骤绑定到特定资产（以防某些资产是可选的），这非常有用。
	 */
	void AsyncEvent(TFunction<void()>&& Callback)
	{
		AsyncEvent(FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/**
	 * 不加载任何内容，而是将此回调插入到回调序列中，以便当异步加载完成时，
	 * 此事件将在序列中的相同点被调用。如果你不希望某个步骤绑定到特定资产（以防某些资产是可选的），这非常有用。
	 */
	UE_API void AsyncEvent(const FSimpleDelegate& Callback);

	/** 刷新所有异步加载请求。 */
	UE_API void StartAsyncLoading();

	/** 取消任何待处理的异步加载。 */
	UE_API void CancelAsyncLoading();

	/** 异步加载当前是否正在进行？ */
	UE_API bool IsAsyncLoadingInProgress() const;

private:
	/**
	 * FLoadingState 是静态映射中为 FAsyncLoadMixin 分配的加载状态，混入类本身不保存加载状态成员，
	 * 我们只在需要时动态创建 FLoadingState，并在不需要时销毁它。
	 */
	class FLoadingState : public TSharedFromThis<FLoadingState>
	{
	public:
		FLoadingState(FAsyncLoadMixin& InOwner);
		virtual ~FLoadingState();

		/** 启动异步序列。 */
		void Start();

		/** 取消异步序列。 */
		void CancelAndDestroy();

		void AsyncLoad(FSoftObjectPath SoftObject, const FSimpleDelegate& DelegateToCall);
		void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall);
		void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& PrimaryAssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall);
		void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback);
		void AsyncEvent(const FSimpleDelegate& Callback);

		bool IsLoadingComplete() const { return !IsLoadingInProgress(); }
		bool IsLoadingInProgress() const;
		bool IsLoadingInProgressOrPending() const;
		bool IsPendingDestroy() const;

	private:
		void CancelOnly(bool bDestroying);
		void CancelStartTimer();
		void TryScheduleStart();
		void TryCompleteAsyncLoading();
		void CompleteAsyncLoading();

	private:
		void RequestDestroyThisMemory();
		void CancelDestroyThisMemory(bool bDestroying);

		/** 加载状态的宿主，用于调用混入对象的加载通知。 */
		FAsyncLoadMixin& OwnerRef;

		/**
		 * 我们是否需要预加载捆绑包？如果我们没有预加载捆绑包（这需要你保持流式句柄存在，否则它们会被销毁），
		 * 那么当所有加载完成后，我们可以安全地销毁 FLoadingState。
		 */
		bool bPreloadedBundles = false;

		class FAsyncStep
		{
		public:
			FAsyncStep(const FSimpleDelegate& InUserCallback);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FAsyncCondition>& InCondition);

			~FAsyncStep();

			void ExecuteUserCallback();

			bool IsLoadingInProgress() const
			{
				return !IsComplete();
			}

			bool IsComplete() const;
			void Cancel();

			bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);
			bool IsCompleteDelegateBound() const;

		private:
			FSimpleDelegate UserCallback;
			bool bIsCompletionDelegateBound = false;

			// 可能的异步'事物'
			TSharedPtr<FStreamableHandle> StreamingHandle;
			TSharedPtr<FAsyncCondition> Condition;
		};

		bool bHasStarted = false;

		int32 CurrentAsyncStep = 0;
		TArray<TUniquePtr<FAsyncStep>> AsyncSteps;
		TArray<TUniquePtr<FAsyncStep>> AsyncStepsPendingDestruction;

		FTSTicker::FDelegateHandle StartTimerDelegate;
		FTSTicker::FDelegateHandle DestroyMemoryDelegate;
	};

	UE_API const FLoadingState& GetLoadingStateConst() const;

	UE_API FLoadingState& GetLoadingState();

	UE_API bool HasLoadingState() const;

	UE_API bool IsLoadingInProgressOrPending() const;

private:
	static UE_API TMap<FAsyncLoadMixin*, TSharedRef<FLoadingState>> Loading;
};

/**
 * 有时混入模式并不合适。对象可能需要管理多个任务，每个任务都有独立的异步依赖链和作用域。
 * 对于这些情况，可以使用 FAsyncLoadScope。
 * 
 * 这个类是一个独立的异步依赖处理器，让你可以启动多个加载任务并始终以正确的顺序处理它们，
 * 就像将 FAsyncLoadMixin 与你的类结合使用一样。
 */
class FAsyncLoadScope : public FAsyncLoadMixin
{
public:
	using FAsyncLoadMixin::AsyncLoad;

	using FAsyncLoadMixin::AsyncPreloadPrimaryAssetsAndBundles;

	using FAsyncLoadMixin::AsyncCondition;

	using FAsyncLoadMixin::AsyncEvent;

	using FAsyncLoadMixin::CancelAsyncLoading;

	using FAsyncLoadMixin::StartAsyncLoading;

	using FAsyncLoadMixin::IsAsyncLoadingInProgress;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

enum class EAsyncConditionResult : uint8
{
	TryAgain,
	Complete
};

DECLARE_DELEGATE_RetVal(EAsyncConditionResult, FAsyncConditionDelegate);

/**
 * 异步条件允许你设置自定义原因来暂停异步加载，直到满足某些条件为止。
 */
class FAsyncCondition : public TSharedFromThis<FAsyncCondition>
{
public:
	UE_API FAsyncCondition(const FAsyncConditionDelegate& Condition);
	UE_API FAsyncCondition(TFunction<EAsyncConditionResult()>&& Condition);
	UE_API virtual ~FAsyncCondition();

protected:
	UE_API bool IsComplete() const;
	UE_API bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);

private:
	bool TryToContinue(float DeltaTime);

	FTSTicker::FDelegateHandle RepeatHandle;
	FAsyncConditionDelegate UserCondition;
	FSimpleDelegate CompletionDelegate;

	friend FAsyncLoadMixin;
};

#undef UE_API
