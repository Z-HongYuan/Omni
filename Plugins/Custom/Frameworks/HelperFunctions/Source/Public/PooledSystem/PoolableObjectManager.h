// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "PoolableObjectManager.generated.h"

#define UE_API HELPERFUNCTIONS_API

USTRUCT()
struct FBucket
{
	GENERATED_BODY()

	TArray<TObjectPtr<UObject>> Inactive = {}; // 休眠池
	TArray<TObjectPtr<UObject>> Active = {}; // 活跃池
	int32 MaxSize = -1; // -1 = 无限制
	int32 LivingCount = 0; // 存活数量数
};

/**
 * 使用通用的池化接口
 * 构建的对象池管理器
 */
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

	/*
	 * 通过软引用获取池化对象
	 */
	template <typename T = UObject>
	T* GetObject(const TSoftClassPtr<T>& PoolableSoftClass)
	{
		// 前置检查,软引用是否为空,类是否为空,类是否加载在内存中
		if (PoolableSoftClass.IsNull()) return nullptr;
		UClass* Class = ResolveClass(PoolableSoftClass);
		if (!Class) return nullptr;

		// 查找对应Class的桶
		FBucket& Bucket = Buckets.FindOrAdd(Class);

		// 如果休眠池中有,则取出
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
				// 在Pop移除的同时,减少存活数
				--Bucket.LivingCount;
				Bucket.Inactive.RemoveSwap(Obj);
			}
		}

		// 检查数量已达上限?
		int32 MaxSize = GetMaxSize(Class);
		if (MaxSize > 0 && Bucket.LivingCount >= MaxSize)
		{
			return nullptr; // 达上限，外部自行排队/降级
		}

		// 池内没有的话就新建一个Obj
		if (UObject* NewObj = SpawnForClass(Class))
		{
			++Bucket.LivingCount;
			Bucket.Active.Add(NewObj);
			Activate(NewObj);
			return Cast<T>(NewObj);
		}

		// 以上情况都不适用,则返回空指针,并且报错
		UE_LOG(LogTemp, Error, TEXT("[PoolableObjectManager] Failed to spawn object for class %s"), *Class->GetName());
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "PoolableSoftClass"), Category = "PoolableObjectManager")
	UE_API UObject* GetObject(const TSoftClassPtr<UObject> PoolableSoftClass)
	{
		if (PoolableSoftClass.IsNull()) return nullptr;
		return GetObject<UObject>(PoolableSoftClass);
	}

	/*
	 * 通过软引用归还池化对象
	 */
	UFUNCTION(BlueprintCallable, Category = "PoolableObjectManager")
	UE_API void ReturnToPool(UObject* PoolableObject);

	/*
	 * 获取对象在休眠池中的数量
	 */
	UFUNCTION(BlueprintPure, Category = "PoolableObjectManager")
	UE_API int32 GetCurrentInactiveSize(TSoftClassPtr<UObject> Class);

	/*
	 * 获取对象在活跃池中的数量
	 */
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
