// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "FaceShadowComponent.generated.h"

#define UE_API HELPERFUNCTIONS_API

/**
 * 从头部、前向和右向插槽计算面部朝向，并写入骨骼网格体的自定义基元数据。
 * 网格组件须带有指定 Tag，材质使用相同的数据索引；更换组件后调用 RefreshMeshComponentRef。
 */
UCLASS(MinimalAPI, ClassGroup=(Render), meta=(BlueprintSpawnableComponent))
class UFaceShadowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UFaceShadowComponent();
	UE_API virtual void BeginPlay() override;
	UE_API virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 按组件 Tag 重新查找目标网格体；BeginPlay 会自动调用一次。 */
	UFUNCTION(BlueprintCallable, Category = "SDF")
	UE_API void RefreshMeshComponentRef();

protected:
	/** 用于查找目标骨骼网格体组件的 Tag。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="过滤的MeshTag"))
	FName SkeletalMeshComponentTag = FName("UseFaceShadowComponent");

	/** 面部朝向计算所用的头部原点插槽。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="头部位置命名"))
	FName HeadSlotName = FName("Head");
	/** 与头部原点共同确定前向方向的插槽。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="前向插槽命名"))
	FName ForwardSlotName = FName("SDF_F");
	/** 与头部原点共同确定右向方向的插槽。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="右方插槽命名"))
	FName RightSlotName = FName("SDF_R");

	/** 前向向量在自定义基元数据中的起始索引，占用三个连续通道。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="Forward参数索引"))
	int32 FaceForwardOffsetIndex = 0;
	/** 右向向量在自定义基元数据中的起始索引，占用三个连续通道。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="Right参数索引"))
	int32 FaceRightOffsetIndex = 3;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> TargetMeshComponent = nullptr;

	FVector PreviousForwardDirection = FVector::ZeroVector;
	FVector PreviousRightDirection = FVector::ZeroVector;
};
