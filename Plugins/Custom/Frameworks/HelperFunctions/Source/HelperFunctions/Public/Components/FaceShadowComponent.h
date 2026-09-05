// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "FaceShadowComponent.generated.h"

#define UE_API HELPERFUNCTIONS_API

/*
 * 控制角色脸部SDF阴影的插件
 * 通过获取网格体中的位置信息,设置材质参数值
 * 需要设置全部的FName参数
 * 将在BeginPlay()中调用一次RefreshFaceMaterialInstance(),如果角色更换了模型,请调用此函数以刷新材质实例
 * 
 * 改用了自定义基元数据
 * 设定Mesh上的基元数据
 * 还是需要向网格体添加Tag用于识别
 * 
 * 需要在骨骼网格体组件中启用自定义数据的通道
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

	//更新Mesh引用
	UFUNCTION(BlueprintCallable, Category = "SDF")
	UE_API void RefreshMeshComponentRef();

protected:
	//骨骼网格体组件拥有的Tag,方便检索组件以获取引用,指定的Tag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="过滤的MeshTag"))
	FName SkeletalMeshComponentTag = FName("UseFaceShadowComponent");

	// 头部插槽名字,用于获取位置信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="头部位置命名"))
	FName HeadSlotName = FName("Head");
	// 前方插槽名字,用于获取位置信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="前向插槽命名"))
	FName ForwardSlotName = FName("SDF_F");
	// 右方插槽名字,用于获取位置信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="右方插槽命名"))
	FName RightSlotName = FName("SDF_R");

	// 自定义基元数据偏移索引 SDF_F参数, 在材质中使用MakeVector3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="Forward参数索引"))
	int32 FaceForwardOffsetIndex = 0;
	// 自定义基元数据偏移索引 SDF_R参数, 在材质中使用MakeVector3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta=(DisplayName="Right参数索引"))
	int32 FaceRightOffsetIndex = 3;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> TargetMeshComponent = nullptr;

	FVector PreviousForwardDirection = FVector::ZeroVector;
	FVector PreviousRightDirection = FVector::ZeroVector;
};
