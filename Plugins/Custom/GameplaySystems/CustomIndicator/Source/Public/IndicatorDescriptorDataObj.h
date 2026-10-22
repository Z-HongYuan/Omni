// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "IndicatorDescriptorDataObj.generated.h"

#define UE_API CUSTOMINDICATOR_API

class UIndicatorDescriptorDataObj;
class UIndicatorManagerComponent;

/** 将场景组件或所属 Actor 投影到指示器画布的方式。 */
UENUM(BlueprintType)
enum class EActorCanvasProjectionMode : uint8
{
	/** 投影组件位置或指定插槽位置。 */
	ComponentPoint,
	/** 先在组件世界包围盒中选择锚点，再投影到屏幕。 */
	ComponentBoundingBox,
	/** 先投影组件包围盒，再在屏幕包围盒中选择锚点。 */
	ComponentScreenBoundingBox,
	/** 先在 Actor 世界包围盒中选择锚点，再投影到屏幕。 */
	ActorBoundingBox,
	/** 先投影 Actor 包围盒，再在屏幕包围盒中选择锚点。 */
	ActorScreenBoundingBox
};

USTRUCT(Blueprintable, BlueprintType, MinimalAPI)
struct FIndicatorDescriptorStruct
{
	GENERATED_BODY()

	/** 供指示器控件读取的业务上下文对象。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TObjectPtr<UObject> DataObject;
	/** 提供投影位置和包围盒的场景组件。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TObjectPtr<USceneComponent> SceneComponent;
	/** 投影使用的组件插槽；NAME_None 表示使用组件位置。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	FName ComponentSocketName = NAME_None;
	/** 异步加载并用于显示指示器的控件类。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;
	/** 所属管理组件的弱引用，不延长管理组件的生命周期。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Binding")
	TWeakObjectPtr<UIndicatorManagerComponent> ManagerPtr;

	/** 是否允许显示指示器；仍需关联有效的场景组件。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bVisible = true;
	/** 是否将屏幕外的指示器限制到屏幕边缘。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bClampToScreen = false;
	/** 指示器被限制到屏幕边缘时是否显示方向箭头。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bShowClampToScreenArrow = false;
	/** 关联场景组件无效时，是否自动注销指示器。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bAutoRemoveWhenIndicatorComponentIsNull = false;
	/** 是否按目标真实方向旋转边缘箭头；关闭时使用四个固定方向。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	bool bUse360ClampArrow = false;
	/** 最大绘制距离；超过距离时跳过投影和绘制，0 表示不限制。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Display")
	float MaxDrawDistance = 0.f;

	/** 场景数据到屏幕位置的投影方式。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	EActorCanvasProjectionMode ProjectionMode = EActorCanvasProjectionMode::ComponentPoint;
	/** 包围盒内的归一化锚点，0.5 表示中心。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector BoundingBoxAnchor = FVector(0.5, 0.5, 0.5);
	/** 投影后附加的屏幕空间偏移。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector2D ScreenSpaceOffset = FVector2D(0, 0);
	/** 附加到组件或插槽位置的世界空间偏移。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Offset")
	FVector WorldPositionOffset = FVector(0, 0, 0);

	/** 控件相对投影位置的水平对齐方式。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;
	/** 控件相对投影位置的垂直对齐方式。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;
	/** 绘制优先级；数值较大的指示器后绘制。 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Layout")
	int32 Priority = 0;
};


/**
 * 保存活动指示器的投影、布局和业务上下文，并记录对应控件的弱引用。
 * 实现 IIndicatorWidgetInterface 的控件通过绑定回调接收该描述对象。
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UIndicatorDescriptorDataObj : public UObject
{
	GENERATED_BODY()

public:
	UIndicatorDescriptorDataObj() { ; }

	/** 将指示器投影到屏幕；输出 XY 为屏幕坐标，Z 为到相机的距离。 */
	static bool Project(
		const UIndicatorDescriptorDataObj& IndicatorDescriptor,
		const FSceneViewProjectionData& InProjectionData,
		const FVector2f& ScreenSize,
		FVector& OutScreenPositionWithDepth
	);

	// TODO 更好的组织
	TWeakObjectPtr<UUserWidget> IndicatorWidget;

	/** 返回是否启用了自动注销且关联场景组件已无效。 */
	bool CanAutomaticallyRemove() const { return IndicatorDescriptorParameter.bAutoRemoveWhenIndicatorComponentIsNull && !IsValid(IndicatorDescriptorParameter.SceneComponent); }

	/** 返回场景组件有效且配置允许显示时的可见状态。 */
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	bool GetIsVisible() const { return IsValid(IndicatorDescriptorParameter.SceneComponent) && IndicatorDescriptorParameter.bVisible; }

	/** 首次注册时设置所属管理组件；已有管理器关联时不覆盖。 */
	UE_API void SetIndicatorManagerComponent(UIndicatorManagerComponent* InManager);

	/** 通知所属管理组件注销指示器，不直接销毁描述对象。 */
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	UE_API void UnregisterIndicator();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Indicator")
	FIndicatorDescriptorStruct IndicatorDescriptorParameter;

private:
	friend class SActorIcon;

	TWeakPtr<SWidget> CanvasHost;
};
#undef UE_API
