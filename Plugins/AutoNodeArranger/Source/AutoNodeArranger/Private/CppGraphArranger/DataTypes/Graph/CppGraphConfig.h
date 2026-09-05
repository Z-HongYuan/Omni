// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../Basic/Vector2.h"

enum ECpp_GraphType
{
	BLUEPRINT,
	MATERIAL,
	AI,
	SOUND
};

enum ECpp_ArrangeSelectionType
{
	// one selected node --> arrange connected graph
	// multiple selected nodes --> arrange only selected nodes
	OneForAll,
	// always arrange only selected nodes
	AlwaysSelected,
	// always arrange connected graph
	AlwaysAll,
};

struct CppGraphConfig
{
	static inline double LINE_SCALE = 5;
	static inline const double ALIGN_THRESHOLD_SCALE = 3;

	// ======== ATTRIBUTES ========

	// graphh config

	double execSpacingX = 0.0;

	Vector2 spacing;
	Vector2 commentSpacing;

	// editor config

	bool bUseArrangement = true;
	ECpp_ArrangeSelectionType arrangeSelectionType = ECpp_ArrangeSelectionType::OneForAll;
	bool bGroupAllConnectedGraph = false;
	double lineWidth = 0.0;
	bool bAutoGenerateReroute = true;

	// arrangement config

	bool bCompact = false;
	bool bCenterize = false;
	ECpp_GraphType graphType = ECpp_GraphType::BLUEPRINT;
	bool bUseReroutePlacerY = false;

	// ======== CONSTRUCTOR ========

	CppGraphConfig() = default;
	CppGraphConfig(double execSpacingX_,
		Vector2 spacing_,
		Vector2 commentSpacing_,
		bool bUseArrangement_,
		ECpp_ArrangeSelectionType arrangeSelectionType_,
		bool bGroupAllConnectedGraph_,
		double lineWidth_,
		bool bAutoGenerateReroute_,
		bool bCompact_,
		bool bCenterize_,
		ECpp_GraphType graphType_,
		bool bUseReroutePlacerY_) :
		execSpacingX(execSpacingX_), spacing(spacing_), commentSpacing(commentSpacing_), bUseArrangement(bUseArrangement_),
		arrangeSelectionType(arrangeSelectionType_), bGroupAllConnectedGraph(bGroupAllConnectedGraph_), lineWidth(lineWidth_),
		bAutoGenerateReroute(bAutoGenerateReroute_), bCompact(bCompact_), bCenterize(bCenterize_), graphType(graphType_),
		bUseReroutePlacerY(bUseReroutePlacerY_)
	{
	}

	// ======== METHODS ========

	bool GetIsBlueprintGraph() const { return graphType == ECpp_GraphType::BLUEPRINT; }

	bool GetIsMaterialGraph() const { return graphType == ECpp_GraphType::MATERIAL; }

	bool GetIsAI_Graph() const { return graphType == ECpp_GraphType::AI; }

	bool GetIsSoundGraph() const { return graphType == ECpp_GraphType::SOUND; }

	bool GetIsCompactBlueprintGraph() const { return bCompact && GetIsBlueprintGraph(); }

	bool GetIsMaterialOrSoundGraph() const { return GetIsMaterialGraph() || GetIsSoundGraph(); }
};