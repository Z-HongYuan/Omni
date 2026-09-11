// Copyright © 2026 张鸿源. All Rights Reserved.


#include "PooledSystem/PoolableObjectManager.h"

#include "Engine/World.h"
#include "PooledSystem/PoolableObjectInterface.h"
#include "PooledSystem/PoolableObjectSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PoolableObjectManager)

void UPoolableObjectManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//从开发者设置中解析拥有的类,提前解析防止卡顿
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

	// 如果缓存中没有的话,会被添加进去备用
	FBucket& Bucket = Buckets.FindOrAdd(Class);

	// 从活跃列表移除,没移除成功的话,就意味着已经在休眠池中,那么就不再需要休眠和添加到休眠池中了
	if (Bucket.Active.RemoveSwap(PoolableObject) == 0) return;

	// 休眠
	Deactivate(PoolableObject);

	// 放进休眠池,避免重复
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
	return -1; // 无限制
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
		return GetWorld()->SpawnActor(Class); // 调用方需再设 Transform
	}
	return NewObject<UObject>(this, Class); // Subsystem 作 Outer
}

UClass* UPoolableObjectManager::ResolveClass(const TSoftClassPtr<>& SoftClass)
{
	return SoftClass.IsValid() ? SoftClass.Get() : SoftClass.LoadSynchronous();
}
