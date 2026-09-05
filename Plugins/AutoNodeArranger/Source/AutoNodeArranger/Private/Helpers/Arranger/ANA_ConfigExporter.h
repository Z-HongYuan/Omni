// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../Config/ANA_EditorConfig.h"
#include "../../CppGraphArranger/DataTypes/Graph/CppGraphConfig.h"

class ANA_ConfigExporter
{
public:
	inline static CppGraphConfig ExportConfig(
		const FGraphConfig& graphConfig, EANA_GraphType graphType, bool bCompact, bool bCenterize)
	{
		UANA_EditorConfig& editorConfig = *UANA_EditorConfig::Get();
		return CppGraphConfig(editorConfig.ExecSpacingX,
			bCompact ? Vector2(graphConfig.CompactSpacing.X, graphConfig.CompactSpacing.Y)
					 : Vector2(graphConfig.Spacing.X, graphConfig.Spacing.Y),
			Vector2(graphConfig.CommentSpacing.X, graphConfig.CommentSpacing.Y),
			editorConfig.bUseArrangement,
			static_cast<ECpp_ArrangeSelectionType>(editorConfig.arrangeSelectionType),
			editorConfig.bGroupAllConnectedGraph,
			editorConfig.LineWidth,
			editorConfig.bAutoGenerateReroute,
			bCompact,
			bCenterize,
			static_cast<ECpp_GraphType>(graphType),
			editorConfig.bUseReroutePlacerY);
	}
};
