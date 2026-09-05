// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <map>
#include <set>

#include "../ExtendedCommon.h"
#include "./DataTypes/EditableGraph.h"

static double INVALID_MIN_POS = -1000000.0;

class NodePlacerCommon
{
public:
	static void UpdateCommentBox(size_t commentId, EditableGraph& editableGraph)
	{
		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& comment = graph.commentList[commentId];
		auto& extraComment = extraGraph.extraCommentList[commentId];
		if (extraComment.insideNodeIdSet.empty()) return;
		comment.box = graph.nodeList.at(*extraComment.insideNodeIdSet.begin()).box;
		for (size_t insideNodeId : extraComment.insideNodeIdSet)
		{
			auto& insideNode = graph.nodeList.at(insideNodeId);
			comment.box += insideNode.box;
		}
	}

	static void GetBasicAllNodeToPlace(size_t root, std::list<size_t>& toPlaceNodeIdList, EditableGraph& editableGraph)
	{
		std::set<size_t> rightVisitedNodeIdSet;
		std::set<size_t> leftVisitedNodeIdSet;

		toPlaceNodeIdList.push_back(root);
		for (auto toPlaceNodeIt = toPlaceNodeIdList.cbegin(); toPlaceNodeIt != toPlaceNodeIdList.cend(); toPlaceNodeIt++)
		{
			getNodeToPlace(false, *toPlaceNodeIt, rightVisitedNodeIdSet, toPlaceNodeIdList, editableGraph);
			getNodeToPlace(true, *toPlaceNodeIt, leftVisitedNodeIdSet, toPlaceNodeIdList, editableGraph);
		}
	}

	static bool GetLinkBox(
		Box& result, size_t linkId, const EditableGraph& editableGraph, double alignPosY, bool bLeft, bool bBoth = false)
	{
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraLink = extraGraph.extraLinkList.at(linkId);
		auto& leftNodeBox = extraLink.GetNode(graph, true).box;
		double scaledLineWidth = CppGraphConfig::LINE_SCALE * graphConfig.lineWidth;
		Vector2 leftPos = Vector2(leftNodeBox.Max.x + 1,
			bBoth	? alignPosY - scaledLineWidth * 0.5
			: bLeft ? alignPosY + extraLink.GetPin(graph, true).offset.y
					: leftNodeBox.Min.y + extraLink.GetPin(graph, true).offset.y);
		auto& rightNodeBox = extraLink.GetNode(graph, false).box;
		Vector2 rightPos = Vector2(rightNodeBox.Min.x - 1,
			bBoth	? alignPosY + scaledLineWidth * 0.5
			: bLeft ? rightNodeBox.Min.y + extraLink.GetPin(graph, false).offset.y
					: alignPosY + extraLink.GetPin(graph, false).offset.y);
		result = Box(leftPos.minWith(rightPos), leftPos.maxWith(rightPos));
		if (bBoth) return true;
		auto size = result.getSize();
		if (size.y > CppGraphConfig::ALIGN_THRESHOLD_SCALE * graphConfig.lineWidth) return false;
		result = Box::PosSize(
			result.Min + Vector2(0., 0.5 * (size.y - scaledLineWidth)), Vector2(size.x, scaledLineWidth)); // center the line
		return true;
	}

private:
	static void getNodeToPlace(bool bLeft,
		size_t nodeId,
		std::set<size_t>& visitedNodeIdSet,
		std::list<size_t>& toPlaceNodeIdList,
		EditableGraph& editableGraph)
	{
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		for (auto linkIdList : {&extraNode.GetLinkIdList(bLeft, true), &extraNode.GetLinkIdList(bLeft, false)})
		{
			for (size_t linkId : *linkIdList)
			{
				auto& extraLink = extraGraph.extraLinkList.at(linkId);
				auto& otherNode = extraLink.GetNodeConnectedTo(graph, nodeId);
				if (extraLink.isBroken || contains(visitedNodeIdSet, otherNode.id)) continue;
				toPlaceNodeIdList.push_back(otherNode.id);
				getNodeToPlace(bLeft, otherNode.id, visitedNodeIdSet, toPlaceNodeIdList, editableGraph);
			}
		}
	}
};
