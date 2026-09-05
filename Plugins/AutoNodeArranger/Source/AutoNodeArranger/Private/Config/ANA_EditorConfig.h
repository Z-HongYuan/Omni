// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "UObject/NoExportTypes.h"

#include "ANA_EditorConfig.generated.h"

USTRUCT(BlueprintType)
struct FGraphConfig
{
	GENERATED_BODY()

	// do not use
	FGraphConfig() {}

	FGraphConfig(FVector2D Spacing, FVector2D CompactSpacing, FVector2D CommentSpacing) :
		Spacing(Spacing), CompactSpacing(CompactSpacing), CommentSpacing(CommentSpacing)
	{
	}

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Graph Config")
	FVector2D Spacing;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Graph Config")
	FVector2D CompactSpacing;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Graph Config")
	FVector2D CommentSpacing;
};

USTRUCT()
struct FGraphTypeStr
{
	GENERATED_BODY()

	// do not use
	FGraphTypeStr() {}

	explicit FGraphTypeStr(FString value) : value(value) {}

	UPROPERTY(VisibleAnywhere, DisplayName = "GraphType", Category = "Graph Config")
	FString value;

	friend bool operator==(const FGraphTypeStr& graphType1, const FGraphTypeStr& graphType2)
	{
		return graphType1.value == graphType2.value;
	}
	friend uint32 GetTypeHash(const FGraphTypeStr& graphType) { return GetTypeHash(graphType.value); }
};

UENUM()
enum class EANA_GraphType : uint8
{
	Blueprint,
	Material,
	AI,
	Sound
};

UENUM()
enum class EArrangeSelectionType : uint8
{
	// one selected node --> arrange connected graph
	// multiple selected nodes --> arrange only selected nodes
	OneForAll UMETA(DisplayName = "One For All"),
	// always arrange only selected nodes
	AlwaysSelected UMETA(DisplayName = "Always Selected"),
	// always arrange connected graph
	AlwaysAll UMETA(DisplayName = "Always All"),
};

UCLASS(config = Editor)
class UANA_EditorConfig : public UObject
{
	GENERATED_BODY()

public:
	inline static UANA_EditorConfig* Get() { return GetMutableDefault<UANA_EditorConfig>(); }

	// Value used to enable the new arrangement (disabled by default since unstable)
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bUseBeta = false;

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	EArrangeSelectionType arrangeSelectionType = EArrangeSelectionType::OneForAll;

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bAutoGenerateReroute = true;

	// Special X spacing for exec blueprint
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	float ExecSpacingX = 120.f;

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bGroupAllConnectedGraph = true;

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bInstantArrange = false;

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bProgressiveSelection = true;

	// Offset added to handle overlap
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger|Advanced", DisplayName = "Overlap Offset")
	float LineWidth = 5.f;

	// Value used to disable arrangement in order to report an issue while arranging
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger|Advanced")
	bool bUseArrangement = true;

	// Value used to enable the add of reroute for no link intersection (disabled by default since unstable)
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger|Advanced")
	bool bUseReroutePlacerY = false;

	static inline const float exponent = 2.5f;
	static inline const float normalSpeed = 5.f;
	static inline const float instantSpeed = 1000.f;

	inline static FGraphConfig ComputeGraphConfig(FString graphTypeString, EANA_GraphType& graphType, bool& isForbiddenGraph)
	{
		return Get()->privateComputeGraphConfig(graphTypeString, graphType, isForbiddenGraph);
	}

	inline static bool ContainsGraphConfig(FString graphTypeString)
	{
		return Get()->customGraphConfigMap.Contains(FGraphTypeStr(graphTypeString));
	}
	inline static FGraphConfig GetGraphConfig(FString graphTypeString)
	{
		return Get()->customGraphConfigMap[FGraphTypeStr(graphTypeString)];
	}
	inline static void AddGraphConfig(FString graphTypeString)
	{
		Get()->customGraphConfigMap.Add(FGraphTypeStr(graphTypeString), Get()->DefaultGraphConfig);
	}

	inline static bool CheckShowNewFeatures()
	{
		bool result = Get()->bShowNewFeatures;
		Get()->bShowNewFeatures = false;
		return result;
	}

private:
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	FGraphConfig DefaultGraphConfig = FGraphConfig(FVector2D(50.f, 30.f), FVector2D(30.f, 15.f), FVector2D(24.f, 24.f));

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	FGraphConfig MaterialGraphConfig = FGraphConfig(FVector2D(100.f, 50.f), FVector2D(50.f, 20.f), FVector2D(30.f, 30.f));

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	FGraphConfig AI_GraphConfig = FGraphConfig(FVector2D(70.f, 100.f), FVector2D(30.f, 50.f), FVector2D(35.f, 35.f));

	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger|Advanced", DisplayName = "Custom Graph Configs")
	TMap<FGraphTypeStr, FGraphConfig> customGraphConfigMap;

	// Show the latest release notification
	UPROPERTY(EditAnywhere, config, Category = "Auto Node Arranger")
	bool bShowNewFeatures = false;

	TSet<FString> ForbiddenGraphTypeSet = {"AnimationStateMachineGraph", "EdGraph_ReferenceViewer"};

	TSet<FString> AI_GraphTypeSet = {"BehaviorTreeGraph", "EnvironmentQueryGraph"};

	inline FGraphConfig privateComputeGraphConfig(FString graphTypeString, EANA_GraphType& graphType, bool& isForbiddenGraph)
	{
		FGraphConfig result;
		if (AI_GraphTypeSet.Contains(graphTypeString))
		{
			graphType = EANA_GraphType::AI;
			result = AI_GraphConfig;
		}
		else if (graphTypeString == "MaterialGraph")
		{
			graphType = EANA_GraphType::Material;
			result = MaterialGraphConfig;
		}
		else if (graphTypeString == "SoundCueGraph")
		{
			graphType = EANA_GraphType::Sound;
			result = DefaultGraphConfig;
		}
		else
		{
			graphType = EANA_GraphType::Blueprint;
			result = DefaultGraphConfig;
		}
		isForbiddenGraph = ForbiddenGraphTypeSet.Contains(graphTypeString);
		return ContainsGraphConfig(graphTypeString) ? GetGraphConfig(graphTypeString) : result;
	}
};
