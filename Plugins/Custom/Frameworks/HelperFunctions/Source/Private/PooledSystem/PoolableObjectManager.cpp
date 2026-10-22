// Copyright © 2026 张鸿源. All Rights Reserved.


#include "PooledSystem/PoolableObjectManager.h"

#include "Engine/World.h"
#include "PooledSystem/PoolableObjectInterface.h"
#include "PooledSystem/PoolableObjectSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PoolableObjectManager)

void UPoolableObjectManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 初始化时加载已配置的类，避免首次取用时再同步加载。
	for (const auto& [Class, Settings] : GetDefault<UPoolableObjectSettings>()->DefaultMaxSizes)
	{
		ResolveClass(Class);
	}
}

void UPoolableObjectManager::Deinitialize()
{
	Buckets.Empty();
	Super::Deinitialize();
}

void UPoolableObjectManager::ReturnToPool(UObject* PoolableObject)
{
	if (!PoolableObject) return;
	if (!PoolableObject->Implements<UPoolableObjectInterface>()) return;

	UClass* Class = PoolableObject->GetClass();

	// 为此类型取得或创建池记录。
	FBucket& Bucket = Buckets.FindOrAdd(Class);

	// 仅归还活跃列表中登记的对象，避免重复归还或接收未借出的实例。
	if (Bucket.Active.RemoveSwap(PoolableObject) == 0) return;

	// 调用对象的休眠回调。
	Deactivate(PoolableObject);

	// 将实例放回休眠池，供下次复用。
	Bucket.Inactive.Add(PoolableObject);
}

int32 UPoolableObjectManager::GetCurrentInactiveSize(TSoftClassPtr<UObject> Class)
{
	return Buckets.FindRef(Class.Get()).Inactive.Num();
}

int32 UPoolableObjectManager::GetCurrentActiveSize(TSoftClassPtr<UObject> Class)
{
	return Buckets.FindRef(Class.Get()).Active.Num();
}

void UPoolableObjectManager::SetMaxSize(TSoftClassPtr<UObject> Class, int32 MaxSize)
{
	Buckets.FindOrAdd(Class.Get()).MaxSize = MaxSize;
}

int32 UPoolableObjectManager::GetMaxSize(const UClass* Class)
{
	if (const FBucket* Found = Buckets.Find(Class))
	{
		if (Found->MaxSize > 0) return Found->MaxSize;
	}
	for (auto& [SoftClass, Size] : GetDefault<UPoolableObjectSettings>()->DefaultMaxSizes)
	{
		if (SoftClass.Get() == Class) return Size;
	}
	return -1; // 不限制数量。
}

void UPoolableObjectManager::Deactivate(UObject* Obj)
{
	if (Obj->Implements<UPoolableObjectInterface>())
	{
		Cast<IPoolableObjectInterface>(Obj)->OnPoolDeactivated();
	}
}

void UPoolableObjectManager::Activate(UObject* Obj)
{
	if (Obj->Implements<UPoolableObjectInterface>())
	{
		Cast<IPoolableObjectInterface>(Obj)->OnPoolActivated();
	}
}

UObject* UPoolableObjectManager::SpawnForClass(UClass* Class)
{
	if (Class->IsChildOf<AActor>())
	{
		return GetWorld()->SpawnActor(Class); // 调用方需设置 Transform。
	}
	return NewObject<UObject>(this, Class); // 以此子系统作为 Outer。
}

UClass* UPoolableObjectManager::ResolveClass(const TSoftClassPtr<>& SoftClass)
{
	return SoftClass.IsValid() ? SoftClass.Get() : SoftClass.LoadSynchronous();
}
