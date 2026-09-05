// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <list>
#include <map>
#include <set>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

#define PLACE_LINES
#define PLACE_LOOP_CHILD

class LineOverlapHandler
{
public:
	static void PlaceLines(size_t root, EditableGraph& editableGraph)
	{
		std::list<size_t> toPlaceNodeIdList;
		NodePlacerCommon::GetBasicAllNodeToPlace(root, toPlaceNodeIdList, editableGraph);

		std::set<size_t> toPlaceNodeIdSet(toPlaceNodeIdList.begin(), toPlaceNodeIdList.end());

		std::set<size_t> visitedNodeIdSet;
		std::set<size_t> placedNodeIdSet;
		std::set<size_t> partiallyPlacedNodeIdSet; // reroute chilren whose brother is not placed yet
		std::map<size_t, Box> placedLineMap;	   // key is linkId, value is LineBox
		for (const auto& toPlaceNodeId : toPlaceNodeIdList)
		{
			auto& graph = editableGraph.graph;
			auto& extraGraph = editableGraph.extraGraph;
			auto& extraNode = extraGraph.extraNodeList.at(toPlaceNodeId);
			auto& node = graph.nodeList.at(toPlaceNodeId);
			if (extraNode.isDeleted) continue;

			double alignPosY = node.box.Min.y;
			alignPosY = handleLineOverlap(node.id, placedNodeIdSet, placedLineMap, alignPosY, editableGraph);
			MOVE_NODE(PLACE_LINES, node.id, Vector2(node.box.Min.x, alignPosY));
			placedNodeIdSet.insert(node.id);

			for (auto& loopChildId : extraNode.loopChildIdSet)
				placeLoopChildInY(loopChildId,
					toPlaceNodeId,
					placedNodeIdSet,
					partiallyPlacedNodeIdSet,
					placedLineMap,
					editableGraph,
					toPlaceNodeIdSet);
		}
	}

private:
	static void placeLoopChildInY(size_t childId,
		size_t parentId,
		std::set<size_t>& placedNodeIdSet,
		std::set<size_t>& partiallyPlacedNodeIdSet,
		std::map<size_t, Box>& placedLineMap,
		EditableGraph& editableGraph,
		const std::set<size_t>& toPlaceNodeIdSet)
	{
		if (contains(placedNodeIdSet, childId)) return;
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraChild = extraGraph.extraNodeList.at(childId);
		auto& parentNode = graph.nodeList.at(parentId);
		auto& childNode = graph.nodeList.at(childId);
		double posY = parentNode.box.Max.y + graph.graphConfig.spacing.y;
		auto& onRightChildId = extraChild.isLoopBrotherPlacedOnRight ? childId : extraChild.loopBrotherId;
		size_t firstLeftLinkId = extraGraph.GetFirstLinkId(onRightChildId, {ELinkFlag::Left}, true);
		if (firstLeftLinkId == static_cast<size_t>(-1))
			throw std::runtime_error("firstLeftLinkId is invalid"); // should not happen

		if (contains(partiallyPlacedNodeIdSet, extraChild.loopBrotherId))
		{
			auto& loopBrotherNode = graph.nodeList.at(extraChild.loopBrotherId);
			posY = std::max(posY, loopBrotherNode.box.Min.y);
			double oldPosY = posY + 1;
			Box linkBox;
			while (oldPosY != posY)
			{
				oldPosY = posY;
				if (graphConfig.lineWidth <= 0) continue;
				NodePlacerCommon::GetLinkBox(linkBox, firstLeftLinkId, editableGraph, posY, false, true);
				for (const auto& nodeId : toPlaceNodeIdSet)
				{
					const auto& extraNode = extraGraph.extraNodeList.at(nodeId);
					if (extraNode.isDeleted) continue;
					const auto& node = graph.nodeList.at(nodeId);
					if (linkBox.intersectWith(node.box))
					{
						posY += node.box.Max.y + graphConfig.spacing.y - linkBox.Min.y;
						break;
					}
				}
				if (oldPosY != posY) continue;
				for (auto& [placedLinkId, placedLine] : placedLineMap)
				{
					if (linkBox.intersectWith(placedLine))
					{
						posY += CppGraphConfig::LINE_SCALE * graphConfig.lineWidth;
						break;
					}
				}
			}
			MOVE_NODE(PLACE_LOOP_CHILD, loopBrotherNode.id, Vector2(loopBrotherNode.box.Min.x, posY));
			partiallyPlacedNodeIdSet.erase(extraChild.loopBrotherId);
			placedNodeIdSet.insert(extraChild.loopBrotherId);
			placedLineMap[firstLeftLinkId] = linkBox;
		}
		else
			partiallyPlacedNodeIdSet.insert(childId);
		MOVE_NODE(PLACE_LOOP_CHILD, childNode.id, Vector2(childNode.box.Min.x, posY));
	}

	static double handleLineOverlap(size_t nodeId,
		const std::set<size_t>& placedNodeIdSet,
		std::map<size_t, Box>& placedLineMap,
		double alignPosY,
		const EditableGraph& editableGraph)
	{
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		if (graphConfig.lineWidth <= 0) return alignPosY;

		std::map<size_t, Box> newlyPlacedLineMap;
		bool bIntersect = true;
		while (bIntersect)
		{
			bIntersect = false;
			newlyPlacedLineMap.clear();
			for (auto& linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
			{
				for (size_t linkId : *linkIdList)
				{
					auto& extraLink = extraGraph.extraLinkList.at(linkId);
					auto& otherNode = extraLink.GetNodeConnectedTo(graph, nodeId);
					if (extraLink.isBroken || !contains(placedNodeIdSet, otherNode.id)) continue;
					Box linkBox;
					if (!NodePlacerCommon::GetLinkBox(
							linkBox, linkId, editableGraph, alignPosY, extraLink.GetNode(graph, true).id == nodeId))
						continue; // ignore pins not aligned
					newlyPlacedLineMap[linkId] = linkBox;
					for (auto& [placedLinkId, placedLine] : placedLineMap)
						if (linkId != placedLinkId && linkBox.intersectWith(placedLine))
						{
							bIntersect = true;
							alignPosY += CppGraphConfig::LINE_SCALE * graphConfig.lineWidth;
							break;
						}
				}
				if (bIntersect) break;
			}
		}

		placedLineMap.insert(newlyPlacedLineMap.begin(), newlyPlacedLineMap.end());
		return alignPosY;
	}
};
