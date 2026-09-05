// Copyright © 2026 张鸿源. All Rights Reserved.

#include "AsyncLoadMixin.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogAsyncMixin, Log, All);

TMap<FAsyncLoadMixin*, TSharedRef<FAsyncLoadMixin::FLoadingState>> FAsyncLoadMixin::Loading;

FAsyncLoadMixin::FAsyncLoadMixin()
{
}

FAsyncLoadMixin::~FAsyncLoadMixin()
{
	check(IsInGameThread());

	// 移除加载状态将取消它正在监控的任何待处理加载，
	// 并且不应再接收任何未来的完成回调。
	Loading.Remove(this);
}

const FAsyncLoadMixin::FLoadingState& FAsyncLoadMixin::GetLoadingStateConst() const
{
	check(IsInGameThread());
	return Loading.FindChecked(this).Get();
}

FAsyncLoadMixin::FLoadingState& FAsyncLoadMixin::GetLoadingState()
{
	check(IsInGameThread());

	if (TSharedRef<FLoadingState>* LoadingState = Loading.Find(this))
	{
		return LoadingState->Get();
	}

	return Loading.Add(this, MakeShared<FLoadingState>(*this)).Get();
}

bool FAsyncLoadMixin::HasLoadingState() const
{
	check(IsInGameThread());

	return Loading.Contains(this);
}

void FAsyncLoadMixin::CancelAsyncLoading()
{
	// 如果没有任何待处理的内容，不要创建加载状态。
	if (HasLoadingState())
	{
		GetLoadingState().CancelAndDestroy();
	}
}

bool FAsyncLoadMixin::IsAsyncLoadingInProgress() const
{
	// 如果没有任何待处理的内容，不要创建加载状态。
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgress();
	}

	return false;
}

bool FAsyncLoadMixin::IsLoadingInProgressOrPending() const
{
	if (HasLoadingState())
	{
		return GetLoadingStateConst().IsLoadingInProgressOrPending();
	}

	return false;
}

void FAsyncLoadMixin::AsyncLoad(const FSoftObjectPath& SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPath, DelegateToCall);
}

void FAsyncLoadMixin::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncLoad(SoftObjectPaths, DelegateToCall);
}

void FAsyncLoadMixin::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	GetLoadingState().AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, DelegateToCall);
}

void FAsyncLoadMixin::AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncCondition(Condition, Callback);
}

void FAsyncLoadMixin::AsyncEvent(const FSimpleDelegate& Callback)
{
	GetLoadingState().AsyncEvent(Callback);
}

void FAsyncLoadMixin::StartAsyncLoading()
{
	// 如果我们实际上没有任何加载状态（因为他们没有排队任何要加载的内容），
	// 就立即通过调用回调来开始和完成操作，没有必要分配内存然后再释放它。
	if (IsLoadingInProgressOrPending())
	{
		GetLoadingState().Start();
	}
	else
	{
		OnStartedLoading();
		OnFinishedLoading();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncLoadMixin::FLoadingState::FLoadingState(FAsyncLoadMixin& InOwner)
	: OwnerRef(InOwner)
{
}

FAsyncLoadMixin::FLoadingState::~FLoadingState()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_DestroyThisMemoryDelegate);
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Destroy LoadingState (Done)"), this);

	// 如果我们被销毁，需要取消我们正在做的任何事情，并取消任何
	// 待处理的销毁 - 因为我们已经在退出的路上了。
	CancelOnly(/*bDestroying*/true);
	CancelDestroyThisMemory(/*bDestroying*/true);
}

void FAsyncLoadMixin::FLoadingState::CancelOnly(bool bDestroying)
{
	if (!bDestroying)
	{
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Cancel"), this);
	}

	CancelStartTimer();

	for (TUniquePtr<FAsyncStep>& Step : AsyncSteps)
	{
		Step->Cancel();
	}

	// 将内存移动到另一个数组，这样我们就不会崩溃。
	// 之前有一个问题，Step 会因为我们在数组上调用 Reset() 而损坏。
	AsyncStepsPendingDestruction = MoveTemp(AsyncSteps);

	bPreloadedBundles = false;
	bHasStarted = false;
	CurrentAsyncStep = 0;
}

void FAsyncLoadMixin::FLoadingState::CancelAndDestroy()
{
	CancelOnly(/*bDestroying*/false);
	RequestDestroyThisMemory();
}

void FAsyncLoadMixin::FLoadingState::CancelDestroyThisMemory(bool bDestroying)
{
	// 如果我们已经计划删除这个内存，我们需要中止它。
	if (IsPendingDestroy())
	{
		if (!bDestroying)
		{
			UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Destroy LoadingState (Canceled)"), this);
		}

		FTSTicker::GetCoreTicker().RemoveTicker(DestroyMemoryDelegate);
		DestroyMemoryDelegate.Reset();
	}
}

void FAsyncLoadMixin::FLoadingState::RequestDestroyThisMemory()
{
	// 如果我们已经 pending 要销毁这个内存，就忽略。
	if (!IsPendingDestroy())
	{
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Destroy LoadingState (Requested)"), this);

		DestroyMemoryDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime)
		{
			// 移除我们使用的所有内存。
			FAsyncLoadMixin::Loading.Remove(&OwnerRef);
			return false;
		}));
	}
}

void FAsyncLoadMixin::FLoadingState::CancelStartTimer()
{
	if (StartTimerDelegate.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(StartTimerDelegate);
		StartTimerDelegate.Reset();
	}
}

void FAsyncLoadMixin::FLoadingState::Start()
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Start (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	// 取消任何待处理的启动加载请求。
	CancelStartTimer();

	// bool bStartingStepFound = false;

	if (!bHasStarted)
	{
		bHasStarted = true;
		OwnerRef.OnStartedLoading();
	}

	TryCompleteAsyncLoading();
}

void FAsyncLoadMixin::FLoadingState::AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] AsyncLoad '%s'"), this, *SoftObjectPath.ToString());

	AsyncSteps.Add(
		MakeUnique<FAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPath, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("AsyncMixin"))
		)
	);

	TryScheduleStart();
}

void FAsyncLoadMixin::FLoadingState::AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall)
{
	{
		const FString& Paths = FString::JoinBy(SoftObjectPaths, TEXT(", "), [](const FSoftObjectPath& SoftObjectPath) { return FString::Printf(TEXT("'%s'"), *SoftObjectPath.ToString()); });
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] AsyncLoad [%s]"), this, *Paths);
	}

	AsyncSteps.Add(
		MakeUnique<FAsyncStep>(
			DelegateToCall,
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftObjectPaths, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, false, false, TEXT("AsyncMixin"))
		)
	);

	TryScheduleStart();
}

void FAsyncLoadMixin::FLoadingState::AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall)
{
	{
		const FString& Assets = FString::JoinBy(AssetIds, TEXT(", "), [](const FPrimaryAssetId& AssetId) { return AssetId.ToString(); });
		const FString& Bundles = FString::JoinBy(LoadBundles, TEXT(", "), [](const FName& LoadBundle) { return LoadBundle.ToString(); });
		UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p]  AsyncPreload Assets [%s], Bundles[%s]"), this, *Assets, *Bundles);
	}

	TSharedPtr<FStreamableHandle> StreamingHandle;

	if (AssetIds.Num() > 0)
	{
		bPreloadedBundles = true;

		constexpr bool bLoadRecursive = true;
		StreamingHandle = UAssetManager::Get().PreloadPrimaryAssets(AssetIds, LoadBundles, bLoadRecursive);
	}

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall, StreamingHandle));

	TryScheduleStart();
}

void FAsyncLoadMixin::FLoadingState::AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] AsyncCondition '0x%p'"), this, &Condition.Get());

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall, Condition));

	TryScheduleStart();
}

void FAsyncLoadMixin::FLoadingState::AsyncEvent(const FSimpleDelegate& DelegateToCall)
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] AsyncEvent"), this);

	AsyncSteps.Add(MakeUnique<FAsyncStep>(DelegateToCall));

	TryScheduleStart();
}

void FAsyncLoadMixin::FLoadingState::TryScheduleStart()
{
	CancelDestroyThisMemory(/*bDestroying*/false);

	// 如果用户忘记启动异步加载，我们将在下一帧开始执行。
	if (!StartTimerDelegate.IsValid())
	{
		StartTimerDelegate = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime)
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncMixin_FLoadingState_TryScheduleStartDelegate);
			Start();
			return false;
		}));
	}
}

bool FAsyncLoadMixin::FLoadingState::IsLoadingInProgress() const
{
	if (AsyncSteps.Num() > 0)
	{
		if (CurrentAsyncStep < AsyncSteps.Num())
		{
			if (CurrentAsyncStep == (AsyncSteps.Num() - 1))
			{
				return AsyncSteps[CurrentAsyncStep]->IsLoadingInProgress();
			}

			// 如果我们知道这是一个有效的索引，但不是最后一个，那么我们知道我们仍在加载，
			// 如果它不是有效索引，我们知道没有加载，或者我们已经超出了所有加载。
			return true;
		}
	}

	return false;
}

bool FAsyncLoadMixin::FLoadingState::IsLoadingInProgressOrPending() const
{
	return StartTimerDelegate.IsValid() || IsLoadingInProgress();
}

bool FAsyncLoadMixin::FLoadingState::IsPendingDestroy() const
{
	return DestroyMemoryDelegate.IsValid();
}

void FAsyncLoadMixin::FLoadingState::TryCompleteAsyncLoading()
{
	// 如果我们在收到此回调时尚未开始，这意味着我们已经完成，
	// 这是在同一帧/栈上完成的其他回调，我们需要避免做任何事情，
	// 直到内存完成删除。
	if (!bHasStarted)
	{
		return;
	}

	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] TryCompleteAsyncLoading - (Current Progress %d/%d)"), this, CurrentAsyncStep + 1, AsyncSteps.Num());

	while (CurrentAsyncStep < AsyncSteps.Num())
	{
		FAsyncStep* Step = AsyncSteps[CurrentAsyncStep].Get();
		if (Step->IsLoadingInProgress())
		{
			if (!Step->IsCompleteDelegateBound())
			{
				UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Step %d - Still Loading (Listening)"), this, CurrentAsyncStep + 1);
				const bool bBound = Step->BindCompleteDelegate(FSimpleDelegate::CreateSP(this, &FLoadingState::TryCompleteAsyncLoading));
				ensureMsgf(bBound, TEXT("This is not intended to return false.  We're checking if it's loaded above, this should definitely return true."));
			}
			else
			{
				UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Step %d - Still Loading (Waiting)"), this, CurrentAsyncStep + 1);
			}

			break;
		}
		else
		{
			UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] Step %d - Completed (Calling User)"), this, CurrentAsyncStep + 1);

			// 总是在调用用户回调之前推进 CurrentAsyncStep，他们可能会添加新工作并尝试再次启动，
			// 所以我们需要为下一步做好准备。
			CurrentAsyncStep++;

			Step->ExecuteUserCallback();
		}
	}

	// 如果我们已完成加载，并且 bHasStarted 仍为 true（意味着这是我们第一次遇到完成请求）
	// 尝试完成。用户回调可能会追加新工作，他们立即启动，立即尝试完成，
	// 这可能会造成我们现在在 TryCompleteAsyncLoading 内部的情况，然后调用 Start，
	// 然后调用 TryCompleteAsyncLoading，所以当我们从栈中返回时，我们需要避免尝试完成
	// 异步加载 N+1 次。
	if (IsLoadingComplete() && bHasStarted)
	{
		CompleteAsyncLoading();
	}
}

void FAsyncLoadMixin::FLoadingState::CompleteAsyncLoading()
{
	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] CompleteAsyncLoading"), this);

	// 标记我们已完成加载。
	if (bHasStarted)
	{
		bHasStarted = false;
		OwnerRef.OnFinishedLoading();
	}

	// 不太可能，但有可能他们在 OnFinishedLoading 回调中开始加载更多东西，
	// 所以再次检查我们是否真的完成了。
	//
	// 注意：我们不会从使用中的内存中删除自己。做预加载捆绑包之类的事情
	// 需要保持流式句柄存活。所以我们让事情保持存活。
	// 
	// 我们不会销毁内存，但我们需要清理任何可能悬挂的东西，
	// 比如捕获的作用域，如完成处理程序。
	if (IsLoadingComplete())
	{
		if (!bPreloadedBundles && !IsLoadingInProgressOrPending())
		{
			// 如果我们已完成所有加载或待处理加载，我们应该清理我们使用的内存。
			// 继续移除拥有 mixin 分配的这个加载状态。
			RequestDestroyThisMemory();
			return;
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncLoadMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback)
	: UserCallback(InUserCallback)
{
}

FAsyncLoadMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle)
	: UserCallback(InUserCallback)
	  , StreamingHandle(InStreamingHandle)
{
}

FAsyncLoadMixin::FLoadingState::FAsyncStep::FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FAsyncCondition>& InCondition)
	: UserCallback(InUserCallback)
	  , Condition(InCondition)
{
}

FAsyncLoadMixin::FLoadingState::FAsyncStep::~FAsyncStep()
{
}

void FAsyncLoadMixin::FLoadingState::FAsyncStep::ExecuteUserCallback()
{
	UserCallback.ExecuteIfBound();
	UserCallback.Unbind();
}

bool FAsyncLoadMixin::FLoadingState::FAsyncStep::IsComplete() const
{
	if (StreamingHandle.IsValid())
	{
		return StreamingHandle->HasLoadCompleted();
	}
	else if (Condition.IsValid())
	{
		return Condition->IsComplete();
	}

	return true;
}

void FAsyncLoadMixin::FLoadingState::FAsyncStep::Cancel()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(FSimpleDelegate());
		StreamingHandle.Reset();
	}
	else if (Condition.IsValid())
	{
		Condition.Reset();
	}

	bIsCompletionDelegateBound = false;
}

bool FAsyncLoadMixin::FLoadingState::FAsyncStep::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		// 太晚了！
		return false;
	}

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->BindCompleteDelegate(NewDelegate);
	}
	else if (Condition)
	{
		Condition->BindCompleteDelegate(NewDelegate);
	}

	bIsCompletionDelegateBound = true;

	return true;
}

bool FAsyncLoadMixin::FLoadingState::FAsyncStep::IsCompleteDelegateBound() const
{
	return bIsCompletionDelegateBound;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

FAsyncCondition::FAsyncCondition(const FAsyncConditionDelegate& Condition)
	: UserCondition(Condition)
{
}

FAsyncCondition::FAsyncCondition(TFunction<EAsyncConditionResult()>&& Condition)
	: UserCondition(FAsyncConditionDelegate::CreateLambda([UserFunction = MoveTemp(Condition)]() mutable { return UserFunction(); }))
{
}

FAsyncCondition::~FAsyncCondition()
{
	FTSTicker::GetCoreTicker().RemoveTicker(RepeatHandle);
}

bool FAsyncCondition::IsComplete() const
{
	if (UserCondition.IsBound())
	{
		const EAsyncConditionResult Result = UserCondition.Execute();
		return Result == EAsyncConditionResult::Complete;
	}

	return true;
}

bool FAsyncCondition::BindCompleteDelegate(const FSimpleDelegate& NewDelegate)
{
	if (IsComplete())
	{
		// 已经完成
		return false;
	}

	CompletionDelegate = NewDelegate;

	if (!RepeatHandle.IsValid())
	{
		RepeatHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FAsyncCondition::TryToContinue), 0.16f);
	}

	return true;
}

bool FAsyncCondition::TryToContinue(float)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FAsyncCondition_TryToContinue);

	UE_LOG(LogAsyncMixin, Verbose, TEXT("[0x%p] AsyncCondition::TryToContinue"), this);

	if (UserCondition.IsBound())
	{
		switch (UserCondition.Execute())
		{
		case EAsyncConditionResult::TryAgain:
			return true;
		case EAsyncConditionResult::Complete:
			RepeatHandle.Reset();
			UserCondition.Unbind();

			CompletionDelegate.ExecuteIfBound();
			CompletionDelegate.Unbind();
			break;
		}
	}

	return false;
}
