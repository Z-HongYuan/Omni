// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "PoolableObjectManager.generated.h"

#define UE_API HELPERFUNCTIONS_API

USTRUCT()
struct FBucket
{
	GENERATED_BODY()

	/** 等待下次复用的对象。 */
	TArray<TObjectPtr<UObject>> Inactive = {};
	/** 已交给调用方使用的对象。 */
	TArray<TObjectPtr<UObject>> Active = {};
	/** 实例数量上限；不大于零时回退到默认配置。 */
	int32 MaxSize = -1;
	/** 当前计入池容量的实例数量。 */
	int32 LivingCount = 0;
};

/** 按类型维护活跃和休眠对象，支持复用实例及限制创建数量。 */
UCLASS(MinimalAPI)
class UPoolableObjectManager : public UWorldSubsystem
{
	GENERATED_BODY()


	UPROPERTY()
	TMap<UClass*, FBucket> Buckets;

public:
	// UWorldSubsystem
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	// End UWorldSubsystem

	/** 通过软类引用获取并激活实例，优先复用休眠对象；达到上限时返回空指针。 */
	template <typename T = UObject>
	T* GetObject(const TSoftClassPtr<T>& PoolableSoftClass)
	{
		// 解析软类引用，必要时同步加载。
		if (PoolableSoftClass.IsNull()) return nullptr;
		UClass* Class = ResolveClass(PoolableSoftClass);
		if (!Class) return nullptr;

		// 按实际类型获取对象池。
		FBucket& Bucket = Buckets.FindOrAdd(Class);

		// 优先复用仍有效的休眠实例。
		while (Bucket.Inactive.Num() > 0)
		{
			UObject* Obj = Bucket.Inactive.Pop();
			if (IsValid(Obj))
			{
				Bucket.Active.Add(Obj);
				Activate(Obj);
				return Cast<T>(Obj);
			}
			else
			{
				// 失效实例不再计入池容量。
				--Bucket.LivingCount;
				Bucket.Inactive.RemoveSwap(Obj);
			}
		}

		// 仅正数上限限制新实例的创建。
		int32 MaxSize = GetMaxSize(Class);
		if (MaxSize > 0 && Bucket.LivingCount >= MaxSize)
		{
			return nullptr; // 达到上限，由调用方排队或降级。
		}

		// 没有可复用实例时创建对象，并在返回前激活。
		if (UObject* NewObj = SpawnForClass(Class))
		{
			++Bucket.LivingCount;
			Bucket.Active.Add(NewObj);
			Activate(NewObj);
			return Cast<T>(NewObj);
		}

		// 创建失败时记录错误，交由调用方处理。
		UE_LOG(LogTemp, Error, TEXT("[PoolableObjectManager] Failed to spawn object for class %s"), *Class->GetName());
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "PoolableSoftClass"), Category = "PoolableObjectManager")
	UE_API UObject* GetObject(const TSoftClassPtr<UObject> PoolableSoftClass)
	{
		if (PoolableSoftClass.IsNull()) return nullptr;
		return GetObject<UObject>(PoolableSoftClass);
	}

	/** 归还活跃实例并调用休眠回调；调用方不应直接销毁待复用对象。 */
	UFUNCTION(BlueprintCallable, Category = "PoolableObjectManager")
	UE_API void ReturnToPool(UObject* PoolableObject);

	/** 获取指定类型的休眠实例数量。 */
	UFUNCTION(BlueprintPure, Category = "PoolableObjectManager")
	UE_API int32 GetCurrentInactiveSize(TSoftClassPtr<UObject> Class);

	/** 获取指定类型的活跃实例数量。 */
	UFUNCTION(BlueprintPure, Category = "PoolableObjectManager")
	UE_API int32 GetCurrentActiveSize(TSoftClassPtr<UObject> Class);

	UE_API void SetMaxSize(TSoftClassPtr<UObject> Class, int32 MaxSize);

private:
	int32 GetMaxSize(const UClass* Class);
	void Deactivate(UObject* Obj);
	void Activate(UObject* Obj);
	UObject* SpawnForClass(UClass* Class);
	UClass* ResolveClass(const TSoftClassPtr<>& SoftClass);
};

#undef UE_API
