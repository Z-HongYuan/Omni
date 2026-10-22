// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameUIExtensionHelper.generated.h"

#define UE_API GAMEUIEXTENSION_API

struct FUIExtensionRequest;
class UGameUIExtensionManager;

/** 扩展点的标签匹配模式。 */
UENUM(BlueprintType)
enum class EUIExtensionPointMatch : uint8
{
	/** 仅匹配完全相同的标签。 */
	ExactMatch,

	/** 允许按标签层级匹配。 */
	PartialMatch
};

/** 扩展的添加或移除通知类型。 */
UENUM(BlueprintType)
enum class EUIExtensionAction : uint8
{
	Added,
	Removed
};

/** 注册到管理器的扩展数据及其匹配条件。 */
struct FUIExtension : TSharedFromThis<FUIExtension>
{
public:
	/** 用于匹配扩展点的标签。 */
	FGameplayTag ExtensionPointTag;
	/** 供接收方决定显示顺序的优先级。 */
	int32 Priority = INDEX_NONE;
	/** 用于限制接收范围的上下文对象，不持有强引用。 */
	TWeakObjectPtr<UObject> ContextObject;
	/** 扩展提供的数据对象，由 UGameUIExtensionManager::AddReferencedObjects 保持 GC 引用。 */
	TObjectPtr<UObject> Data = nullptr;
};

DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, EUIExtensionAction Action, const FUIExtensionRequest& Request);

/** 保存扩展点的接收条件及通知回调。 */
struct FUIExtensionPoint : TSharedFromThis<FUIExtensionPoint>
{
public:
	/** 扩展点的识别标签。 */
	FGameplayTag ExtensionPointTag;
	/** 扩展点要求的上下文对象。 */
	TWeakObjectPtr<UObject> ContextObject;
	/** 扩展点的标签匹配规则。 */
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;
	/** 允许接收的类或接口，由管理器收集 GC 引用。 */
	TArray<TObjectPtr<UClass>> AllowedDataClasses;
	/** 匹配扩展添加或移除时执行的回调。 */
	FExtendExtensionPointDelegate Callback;

	/** 检查上下文及数据类型是否满足当前扩展点的接收条件。 */
	bool DoesExtensionPassContract(const FUIExtension* Extension) const;
};

/** 用于显式注销扩展点的句柄；销毁句柄不会自动注销。 */
USTRUCT(BlueprintType)
struct FUIExtensionPointHandle
{
	GENERATED_BODY()

public:
	FUIExtensionPointHandle() { ; }

	UE_API void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionPointHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionPointHandle& Other) const { return !operator==(Other); }

	friend uint32 GetTypeHash(const FUIExtensionPointHandle& Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UGameUIExtensionManager> ExtensionSource;

	TSharedPtr<FUIExtensionPoint> DataPtr;

	friend UGameUIExtensionManager;

	FUIExtensionPointHandle(UGameUIExtensionManager* InExtensionSource, const TSharedPtr<FUIExtensionPoint>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) { ; }
};

template <>
struct TStructOpsTypeTraits<FUIExtensionPointHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionPointHandle>
{
	enum
	{
		/** 允许蓝图复制包含共享指针的句柄。 */
		WithCopy = true,
		WithIdenticalViaEquality = true,
	};
};

/** 用于显式注销扩展的句柄；销毁句柄不会自动注销。 */
USTRUCT(BlueprintType)
struct FUIExtensionHandle
{
	GENERATED_BODY()

public:
	FUIExtensionHandle() { ; }

	UE_API void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionHandle& Other) const { return !operator==(Other); }

	friend FORCEINLINE uint32 GetTypeHash(FUIExtensionHandle Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UGameUIExtensionManager> ExtensionSource;

	TSharedPtr<FUIExtension> DataPtr;

	friend UGameUIExtensionManager;

	FUIExtensionHandle(UGameUIExtensionManager* InExtensionSource, const TSharedPtr<FUIExtension>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) { ; }
};

template <>
struct TStructOpsTypeTraits<FUIExtensionHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionHandle>
{
	enum
	{
		/** 允许蓝图复制包含共享指针的句柄。 */
		WithCopy = true,
		WithIdenticalViaEquality = true,
	};
};


/** 传给扩展点回调的扩展快照，包含句柄、数据和上下文。 */
USTRUCT(BlueprintType)
struct FUIExtensionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FUIExtensionHandle ExtensionHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ExtensionPointTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> Data = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> ContextObject = nullptr;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FExtendExtensionPointDynamicDelegate, EUIExtensionAction, Action, const FUIExtensionRequest&, ExtensionRequest);

UCLASS(MinimalAPI)
class UUIExtensionHandleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIExtensionHandleFunctions() { ; }

	/** 使用注册句柄注销扩展。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API void Unregister(UPARAM(ref) FUIExtensionHandle& Handle);

	/** 检查句柄是否仍持有扩展记录，不代表记录仍在管理器中注册。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API bool IsValid(UPARAM(ref) FUIExtensionHandle& Handle);
};

UCLASS(MinimalAPI)
class UUIExtensionPointHandleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIExtensionPointHandleFunctions() { ; }

	/** 使用注册句柄注销扩展点。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API void Unregister(UPARAM(ref) FUIExtensionPointHandle& Handle);

	/** 检查句柄是否仍持有扩展点记录，不代表记录仍在管理器中注册。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API bool IsValid(UPARAM(ref) FUIExtensionPointHandle& Handle);
};

#undef UE_API
