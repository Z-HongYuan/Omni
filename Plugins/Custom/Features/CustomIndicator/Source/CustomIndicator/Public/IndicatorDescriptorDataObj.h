// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "IndicatorDescriptorDataObj.generated.h"

#define UE_API CUSTOMINDICATOR_API

class UIndicatorDescriptorDataObj;
class UIndicatorManagerComponent;

/*
 * Actor所使用的投影模式,在UI中
 */
UENUM(BlueprintType)
enum class EActorCanvasProjectionMode : uint8
{
	//组件点
	ComponentPoint,
	//组件边界盒
	ComponentBoundingBox,
	//组件屏幕边界盒
	ComponentScreenBoundingBox,
	//演员边界盒
	ActorBoundingBox,
	//演员屏幕边界盒
	ActorScreenBoundingBox
};

USTRUCT(Blueprintable, BlueprintType, MinimalAPI)
struct FIndicatorDescriptorStruct
{
	GENERATED_BODY()

	// 绑定的 Obj, 就是此说明符的上下文Obj,如果Widget实现了接口,那么可以通过这个指针进行获取数据
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TObjectPtr<UObject> DataObject;
	// 绑定的场景组件, 提供位置信息
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TObjectPtr<USceneComponent> SceneComponent;
	// 绑定的组件Socket, 提供位置信息, 如果没有的话, 则使用组件的位置
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	FName ComponentSocketName = NAME_None;
	// 绑定使用的UI控件, 用于展示指示器的控件
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;
	// 绑定的管理器, 此指示器数据所属的管理器
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TWeakObjectPtr<UIndicatorManagerComponent> ManagerPtr;

	//是否可见, 默认可见
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bVisible = true;
	//是否限制到屏幕, 默认不限制
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bClampToScreen = false;
	//是否限制箭头到屏幕, 默认不显示箭头
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bShowClampToScreenArrow = false;
	//是否自动删除，当关联的组件为空时自动删除, 默认不自动删除
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bAutoRemoveWhenIndicatorComponentIsNull = false;
	//是否启用360°旋转的夹取箭头, 默认关闭, 使用固定的四方向箭头
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bUse360ClampArrow = false;
	//最大绘制距离, 超出此距离的指示器不投影不绘制, 默认0 = 不限制
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	float MaxDrawDistance = 0.f;

	//投影模式
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	EActorCanvasProjectionMode ProjectionMode = EActorCanvasProjectionMode::ComponentPoint;
	//边界盒锚点
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector BoundingBoxAnchor = FVector(0.5, 0.5, 0.5);
	//屏幕空间偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector2D ScreenSpaceOffset = FVector2D(0, 0);
	//世界空间偏移
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector WorldPositionOffset = FVector(0, 0, 0);

	//水平对齐
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;
	//垂直对齐
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;
	//UI中的优先级
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	int32 Priority = 0;
};


/**
 * 描述并控制一个活动指示器,本体为指示器的配置数据
 * 如果指示器Icon实现了IIndicatorWidgetInterface，那么可以通过接口获取到此指示器数据上下文
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UIndicatorDescriptorDataObj : public UObject
{
	GENERATED_BODY()

public:
	UIndicatorDescriptorDataObj() { ; }

	static bool Project(
		const UIndicatorDescriptorDataObj& IndicatorDescriptor, // 指示器配置
		const FSceneViewProjectionData& InProjectionData, // 相机投影数据（从 LocalPlayer 拿）
		const FVector2f& ScreenSize, // 当前屏幕分辨率
		FVector& OutScreenPositionWithDepth // 输出：XY = 屏幕坐标, Z = 距相机距离
	);

	// TODO 更好的组织
	TWeakObjectPtr<UUserWidget> IndicatorWidget;

	// 是否可以自动删除，当关联的组件为空时自动删除
	bool CanAutomaticallyRemove() const { return IndicatorDescriptorParameter.bAutoRemoveWhenIndicatorComponentIsNull && !IsValid(IndicatorDescriptorParameter.SceneComponent); }

	// 获取可见性
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	bool GetIsVisible() const { return IsValid(IndicatorDescriptorParameter.SceneComponent) && IndicatorDescriptorParameter.bVisible; }

	// 设置指示器管理器组件
	UE_API void SetIndicatorManagerComponent(UIndicatorManagerComponent* InManager);

	// 删除指示器
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	UE_API void UnregisterIndicator();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Indicator")
	FIndicatorDescriptorStruct IndicatorDescriptorParameter;

private:
	friend class SActorIcon;

	TWeakPtr<SWidget> CanvasHost;
};
#undef UE_API
