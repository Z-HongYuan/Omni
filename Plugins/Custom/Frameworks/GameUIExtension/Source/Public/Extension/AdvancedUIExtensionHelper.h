// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdvancedUIExtensionHelper.generated.h"

#define UE_API GAMEUIEXTENSION_API

struct FUIExtensionRequest;
class UAdvancedUIExtensionManager;

// 拓展点匹配模式
UENUM(BlueprintType)
enum class EUIExtensionPointMatch : uint8
{
	// 完全匹配 (A.B == A.B != A.B.C)
	ExactMatch,

	// 模糊匹配 (e.g., A.B == A.B == A.B.C)
	PartialMatch
};

// 拓展点匹配规则
UENUM(BlueprintType)
enum class EUIExtensionAction : uint8
{
	Added,
	Removed
};

/*
 * 向拓展点放置的拓展数据
 */
struct FUIExtension : TSharedFromThis<FUIExtension>
{
public:
	// 查询拓展点的Tag
	FGameplayTag ExtensionPointTag;
	// 拓展在拓展点内的排序
	int32 Priority = INDEX_NONE;
	// 拓展提供的上下文对象 (用于筛选)
	TWeakObjectPtr<UObject> ContextObject;
	// 保持存活 UUIExtensionSubsystem::AddReferencedObjects
	// 拓展提供的实际数据对象
	TObjectPtr<UObject> Data = nullptr;
};

DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, EUIExtensionAction Action, const FUIExtensionRequest& Request);

/**
 * 拓展点的接收器
 */
struct FUIExtensionPoint : TSharedFromThis<FUIExtensionPoint>
{
public:
	// 拓展点的识别Tag
	FGameplayTag ExtensionPointTag;
	// 拓展点允许的上下文
	TWeakObjectPtr<UObject> ContextObject;
	// 拓展点的匹配规则
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;
	// 拓展点允许接收的数据类
	TArray<TObjectPtr<UClass>> AllowedDataClasses;
	// 拓展点更改的委托
	FExtendExtensionPointDelegate Callback;

	// 测试扩展和当前扩展点是否匹配
	bool DoesExtensionPassContract(const FUIExtension* Extension) const;
};

/**
 * 用于拓展点的句柄,可以用于注销拓展点
 */
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
	TWeakObjectPtr<UAdvancedUIExtensionManager> ExtensionSource;

	TSharedPtr<FUIExtensionPoint> DataPtr;

	friend UAdvancedUIExtensionManager;

	FUIExtensionPointHandle(UAdvancedUIExtensionManager* InExtensionSource, const TSharedPtr<FUIExtensionPoint>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) { ; }
};

template <>
struct TStructOpsTypeTraits<FUIExtensionPointHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionPointHandle>
{
	enum
	{
		WithCopy = true, // 这可确保在BP中正确复制不透明类型
		WithIdenticalViaEquality = true,
	};
};

/**
 * 用于拓展数据的句柄,可以用于注销拓展
 */
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
	TWeakObjectPtr<UAdvancedUIExtensionManager> ExtensionSource;

	TSharedPtr<FUIExtension> DataPtr;

	friend UAdvancedUIExtensionManager;

	FUIExtensionHandle(UAdvancedUIExtensionManager* InExtensionSource, const TSharedPtr<FUIExtension>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) { ; }
};

template <>
struct TStructOpsTypeTraits<FUIExtensionHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionHandle>
{
	enum
	{
		WithCopy = true, // 这可确保在BP中正确复制不透明类型
		WithIdenticalViaEquality = true,
	};
};


/**
 * 用于在 Manager 中注册一个拓展所使用的请求
 */
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

	// 传入注册拓展的句柄,用于注销
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API void Unregister(UPARAM(ref) FUIExtensionHandle& Handle);

	// 传入注册拓展的句柄,用于判断是否有效
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API bool IsValid(UPARAM(ref) FUIExtensionHandle& Handle);
};

UCLASS(MinimalAPI)
class UUIExtensionPointHandleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIExtensionPointHandleFunctions() { ; }

	// 传入注册拓展点的句柄,用于注销
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API void Unregister(UPARAM(ref) FUIExtensionPointHandle& Handle);

	// 传入注册拓展点的句柄,用于判断是否有效
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static UE_API bool IsValid(UPARAM(ref) FUIExtensionPointHandle& Handle);
};

#undef UE_API
