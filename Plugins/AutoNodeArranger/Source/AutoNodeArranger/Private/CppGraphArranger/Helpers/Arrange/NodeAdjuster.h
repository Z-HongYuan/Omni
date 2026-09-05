// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <functional>
#include <set>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

class NodeAdjuster
{
public:
	// update ExtraComment::rightEdgeOutsideFirstRightNodeIdSet to ensure no unwanted overlap
	// return true if any node is adjusted
	static bool AdjustNodes(const std::set<size_t>& nodeIdSet, EditableGraph& editableGraph)
	{
		bool bNodeAdjusted = false;
		std::set<size_t> commentIdToIgnoreSet;
		for (size_t i = 0; i < editableGraph.graph.commentList.size(); i++)
		{
			NodePlacerCommon::UpdateCommentBox(i, editableGraph);
			auto& comment = editableGraph.graph.commentList[i];
			auto& extraComment = editableGraph.extraGraph.extraCommentList[i];
			for (size_t nodeId : nodeIdSet)
			{
				auto& node = editableGraph.graph.nodeList[nodeId];
				if (contains(extraComment.insideNodeIdSet, node.id)) continue;
				auto& extraNode = editableGraph.extraGraph.extraNodeList[node.id];
				// check node not inside an ignored comment
				// and check node intersect with comment that it should not
				if (intersection(commentIdToIgnoreSet, extraNode.insideCommentIdSet).empty()
					&& node.box.intersectWith(comment.box))
				{
					bNodeAdjusted = true;
					extraComment.rightEdgeOutsideFirstRightNodeIdSet.insert(node.id);
					extraNode.leftEdgeOutsideCommentIdSet.insert(comment.id);
					commentIdToIgnoreSet.insert(comment.id);
					for (auto insideCommentId : extraNode.insideCommentIdSet)
					{
						auto& extraInsideComment = editableGraph.extraGraph.extraCommentList[insideCommentId];
						for (auto insideNodeId : extraInsideComment.insideNodeIdSet)
						{
							auto& extraInsideNode = editableGraph.extraGraph.extraNodeList[insideNodeId];
							extraComment.rightEdgeOutsideFirstRightNodeIdSet.insert(insideNodeId);
							extraInsideNode.leftEdgeOutsideCommentIdSet.insert(comment.id);
						}
					}
				}
			}
		}
		return bNodeAdjusted;
	}
};
