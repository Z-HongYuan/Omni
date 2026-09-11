// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Math/BoxSphereBounds.h"

#include "UObject/ObjectPtr.h"
#include "PocketLevelInstance.generated.h"

#define UE_API POCKETWORLDS_API

class UPocketLevelSubsystem;

class ULevelStreamingDynamic;
class ULocalPlayer;
class UPocketLevel;
class UPocketLevelInstance;
class UWorld;
struct FFrame;

// 口袋世界实例事件的委托签名，参数为该实例自身。
DECLARE_MULTICAST_DELEGATE_OneParam(FPocketLevelInstanceEvent, UPocketLevelInstance*);

/**
 * 一个口袋世界的流送实例句柄，负责加载/卸载以及“加载完成”回调。
 */
UCLASS(MinimalAPI, Within = PocketLevelSubsystem, BlueprintType)
class UPocketLevelInstance : public UObject
{
	GENERATED_BODY()

public:
	UE_API UPocketLevelInstance() = default;

	// 销毁时卸载关卡、解绑流送回调，且不做阻塞式卸载。
	UE_API virtual void BeginDestroy() override;

	// 让口袋世界加载并可见。
	UE_API void StreamIn();
	// 让口袋世界隐藏并卸载，之后仍可再次 StreamIn。
	UE_API void StreamOut();

	// 注册“口袋世界已就绪（关卡已显示）”回调。若当前已就绪会立即同步执行一次，返回句柄用于反注册；未初始化时返回无效句柄。
	UE_API FDelegateHandle AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback);
	// 反注册之前添加的就绪回调。
	UE_API void RemoveReadyCallback(FDelegateHandle CallbackToRemove);

	// 返回该实例所属的世界。
	virtual class UWorld* GetWorld() const override { return World; }

private:
	// 由子系统调用：按本地玩家、口袋世界资产与生成点建立动态流送。
	UE_API bool Initialize(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector SpawnPoint);

	// 关卡加载完成回调：把关卡标记为仅客户端可见，并把其中 Actor 的拥有者设为本地玩家控制器。
	UFUNCTION()
	UE_API void HandlePocketLevelLoaded();

	// 关卡显示回调：广播就绪事件。
	UFUNCTION()
	UE_API void HandlePocketLevelShown();

private:
	// 拥有该实例的本地玩家。
	UPROPERTY()
	TObjectPtr<ULocalPlayer> LocalPlayer;

	// 对应的口袋世界数据资产。
	UPROPERTY()
	TObjectPtr<UPocketLevel> PocketLevel;

	// 该实例所在的世界。
	UPROPERTY()
	TObjectPtr<UWorld> World;

	// 动态流送关卡对象。
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingPocketLevel;

	// 关卡显示（就绪）时广播的多播委托。
	FPocketLevelInstanceEvent OnReadyEvent;

	// 该实例在世界中的包围球范围，用于与其他实例错开、避免重叠。
	FBoxSphereBounds Bounds;

	// 允许子系统访问私有的 Initialize。
	friend class UPocketLevelSubsystem;
};

#undef UE_API
