// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "./DataTypes/Graph/CppGraph.h"
#include "./DataTypes/Instruction/SelectInstruction.h"
#include "./Helpers/Arrange/DataTypes/ExtraGraph.h"
#include "./Helpers/Arrange/DataTypes/ExtraNode.h"
#include "./Helpers/ExtendedCommon.h"

class NodeSelector
{
public:
	explicit NodeSelector(const CppGraph& graph_) : graph(&graph_), extraGraph(graph_, false) {}

	SelectInstruction SelectConnected(bool bProgressiveSelection)
	{
		selectInstruction.nodeIdSet = std::set<size_t>(graph->selectedNodeIdList.begin(), graph->selectedNodeIdList.end());
		selectInstruction.commentIdSet
			= std::set<size_t>(graph->selectedCommentIdList.begin(), graph->selectedCommentIdList.end());

		selectAllCommentsContainingOnlySelectedNodes();
		bool bNewSelection = true;
		while (bNewSelection)
		{
			bNewSelection = false;
			// step 1
			if (selectAllNodesContainedBySelectedComments())
			{
				if (bProgressiveSelection) break;
				bNewSelection = true;
			}
			// step 2
			if (selectAllNodesConnectedToSelectedNodesInsideSameComments())
			{
				if (bProgressiveSelection) break;
				bNewSelection = true;
				selectAllCommentsContainingOnlySelectedNodes();
			}
			// step 3
			if (selectAllNodesConnectedToSelectedNodes())
			{
				if (bProgressiveSelection) break;
				bNewSelection = true;
				selectAllCommentsContainingOnlySelectedNodes();
			}
			// step 4
			if (selectAllCommentsContainingSelectedNodes())
			{
				if (bProgressiveSelection) break;
				bNewSelection = true;
			}
		}
		selectAllCommentsContainingOnlySelectedNodes();

		return selectInstruction;
	};

private:
	const CppGraph* graph;
	SelectInstruction selectInstruction;
	ExtraGraph extraGraph;

	void selectAllCommentsContainingOnlySelectedNodes()
	{
		for (const auto& [_, extraComment] : extraGraph.extraCommentList)
		{
			if (contains(selectInstruction.commentIdSet, extraComment.id)) continue; // comment already selected
			bool bAllNodeSelected = true;
			for (const auto& nodeId : extraComment.insideNodeIdSet)
			{
				if (!contains(selectInstruction.nodeIdSet, nodeId))
				{
					bAllNodeSelected = false;
					break;
				}
			}
			if (bAllNodeSelected)
			{
				// select comment since all contained node selected
				selectInstruction.commentIdSet.insert(extraComment.id);
			}
		}
	}

	// step 1
	bool selectAllNodesContainedBySelectedComments()
	{
		bool bNewSelection = false;
		for (const auto& selectedCommentId : selectInstruction.commentIdSet)
		{
			for (const auto& nodeId : extraGraph.extraCommentList[selectedCommentId].insideNodeIdSet)
			{
				if (!contains(selectInstruction.nodeIdSet, nodeId))
				{
					// select node since contained by selected comment
					selectInstruction.nodeIdSet.insert(nodeId);
					bNewSelection = true;
				}
			}
		}
		return bNewSelection;
	}

	// step 2
	bool selectAllNodesConnectedToSelectedNodesInsideSameComments()
	{
		std::set<size_t> visitedNodeIdSet;
		auto toVisitNodeIdList = std::vector<size_t>(selectInstruction.nodeIdSet.begin(), selectInstruction.nodeIdSet.end());
		bool bNewSelection = false;

		for (size_t i = 0; i < toVisitNodeIdList.size(); ++i)
		{
			auto nodeId = toVisitNodeIdList[i];
			if (contains(visitedNodeIdSet, nodeId)) continue;
			visitedNodeIdSet.insert(nodeId);
			auto& node = extraGraph.extraNodeList.at(nodeId);
			auto& insideCommentIdSet = extraGraph.extraNodeList.at(nodeId).insideCommentIdSet;
			for (const auto& linkIdList : extraGraph.extraNodeList.at(nodeId).GetLinkIdListList({ELinkFlag::All}))
				for (const auto& linkId : *linkIdList)
				{
					auto& otherNode = extraGraph.extraLinkList.at(linkId).GetNodeConnectedTo(*graph, nodeId);
					// ignore if already selected
					if (contains(selectInstruction.nodeIdSet, otherNode.id)) continue;
					// get otherNode containingCommentIdSet
					auto otherInsideCommentIdSet = extraGraph.extraNodeList.at(otherNode.id).insideCommentIdSet;
					// check if set are equal
					if (insideCommentIdSet != otherInsideCommentIdSet) continue;
					// add to toVisitNodeIdList
					toVisitNodeIdList.push_back(otherNode.id);
					// select connected node
					selectInstruction.nodeIdSet.insert(otherNode.id);
					bNewSelection = true;
				}
		}
		return bNewSelection;
	}

	// step 3
	bool selectAllNodesConnectedToSelectedNodes()
	{
		std::set<size_t> visitedNodeIdSet;
		auto toVisitNodeIdList = std::vector<size_t>(selectInstruction.nodeIdSet.begin(), selectInstruction.nodeIdSet.end());
		bool bNewSelection = false;

		for (size_t i = 0; i < toVisitNodeIdList.size(); ++i)
		{
			auto nodeId = toVisitNodeIdList[i];
			if (contains(visitedNodeIdSet, nodeId)) continue;
			visitedNodeIdSet.insert(nodeId);
			for (const auto& linkIdList : extraGraph.extraNodeList.at(nodeId).GetLinkIdListList({ELinkFlag::All}))
				for (const auto& linkId : *linkIdList)
				{
					auto& otherNode = extraGraph.extraLinkList.at(linkId).GetNodeConnectedTo(*graph, nodeId);
					// add to toVisitNodeIdList
					toVisitNodeIdList.push_back(otherNode.id);
					// check if already selected
					if (contains(selectInstruction.nodeIdSet, otherNode.id)) continue;
					// select connected node
					selectInstruction.nodeIdSet.insert(otherNode.id);
					bNewSelection = true;
				}
		}

		return bNewSelection;
	}

	// step 4
	bool selectAllCommentsContainingSelectedNodes()
	{
		bool bNewSelection = false;
		for (const auto& [_, extraComment] : extraGraph.extraCommentList)
		{
			if (contains(selectInstruction.commentIdSet, extraComment.id)) continue; // comment already selected
			for (const auto& nodeId : extraComment.insideNodeIdSet)
			{
				if (contains(selectInstruction.nodeIdSet, nodeId))
				{
					// select comment since containing selected node
					selectInstruction.commentIdSet.insert(extraComment.id);
					bNewSelection = true;
					break;
				}
			}
		}
		return bNewSelection;
	}
};
