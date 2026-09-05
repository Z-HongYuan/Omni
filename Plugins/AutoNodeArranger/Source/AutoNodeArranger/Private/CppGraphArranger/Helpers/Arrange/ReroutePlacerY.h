// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <list>
#include <map>
#include <set>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

#define CREATE_REROUTES
#define GET_LINK_TO_BREAK

class ReroutePlacerY
{
public:
	static void PlaceRerouteInY(size_t root, EditableGraph& editableGraph)
	{
		if (!editableGraph.graph.graphConfig.bAutoGenerateReroute) return;

		std::list<size_t> toPlaceNodeIdList;
		NodePlacerCommon::GetBasicAllNodeToPlace(root, toPlaceNodeIdList, editableGraph);

		std::set<size_t> visitedNodeIdSet;
		std::set<size_t> doneNodeIdSet;
		std::map<size_t, std::list<size_t>> leftPinIdToRerouteListMap;

		for (const auto& toPlaceNodeId : toPlaceNodeIdList)
		{
			if (contains(doneNodeIdSet, toPlaceNodeId)) continue;
			doneNodeIdSet.insert(toPlaceNodeId);

			auto& graph = editableGraph.graph;
			auto& extraGraph = editableGraph.extraGraph;
			auto& extraNode = extraGraph.extraNodeList.at(toPlaceNodeId);
			if (extraNode.isDeleted) continue;

			for (auto linkIdList : extraNode.GetLinkIdListList({ELinkFlag::Right}))
			{
				for (size_t linkId : *linkIdList)
				{
					editableGraph.CheckTimeout();
					auto& extraLink = extraGraph.extraLinkList.at(linkId);
					if (extraLink.isBroken) continue;
					auto& rightNode = extraLink.GetNode(graph, false);
					if (!contains(toPlaceNodeIdList, rightNode.id)) continue;

					auto& link = graph.linkList.at(linkId);
					size_t linkIdToBreak = getLinkToBreak(link, editableGraph, leftPinIdToRerouteListMap);

					createReroutesIfNeeded(
						link.leftPinId, linkIdToBreak, editableGraph, toPlaceNodeIdList, leftPinIdToRerouteListMap);
				}
			}
		}
	}

private:
	enum class ECreateReroute
	{
		Left,
		Right,
		Both
	};

	// check if line intersects with any of the nodes, if so the lines is moved until no intersection occurs
	static bool isLineIntersect(
		size_t linkId, Box& linkBox, const EditableGraph& editableGraph, const std::list<size_t>& nodeIdList)
	{
		auto& extraGraph = editableGraph.extraGraph;
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;

		auto& extraLink = extraGraph.extraLinkList.at(linkId);
		auto& leftNodeBox = extraLink.GetNode(graph, true).box;
		auto& rightNodeBox = extraLink.GetNode(graph, false).box;

		bool bAtLeastOneIntersect = false;
		bool bIntersect = true;
		while (bIntersect)
		{
			bIntersect = false;
			for (const auto& nodeId : nodeIdList)
			{
				const auto& extraNode = extraGraph.extraNodeList.at(nodeId);
				if (extraNode.isDeleted) continue;
				const auto& node = graph.nodeList.at(nodeId);
				if (linkBox.intersectWith(node.box))
				{
					bIntersect = bAtLeastOneIntersect = true;
					linkBox.Min.x = leftNodeBox.Max.x + (extraLink.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x);
					linkBox.Max.x = rightNodeBox.Min.x - (extraLink.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x);
					if (linkBox.Min.x > linkBox.Max.x) linkBox.Max.x = rightNodeBox.Min.x; // no space for 2 reroutes
					linkBox.Min.y = linkBox.Max.y = std::max(linkBox.Min.y, linkBox.Max.y);
					linkBox = linkBox.offsetBox(Vector2(0., node.box.Max.y + graphConfig.spacing.y - linkBox.Min.y));
				}
			}
		}
		return bAtLeastOneIntersect;
	}

	// get the link that is the start of the reroute chain
	// then return the first link where the right node is to the right of the right node of the start link
	static size_t getLinkToBreak(
		const CppLink& link, EditableGraph& editableGraph, const std::map<size_t, std::list<size_t>>& leftPinIdToRerouteListMap)
	{
		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;

		auto& extraLink = extraGraph.extraLinkList.at(link.id);
		auto& rightNodeBox = extraLink.GetNode(graph, false).box;

		auto it = leftPinIdToRerouteListMap.find(link.leftPinId);
		if (it == leftPinIdToRerouteListMap.end()) return link.id;
		for (size_t rerouteId : it->second)
		{
			auto& rerouteBox = graph.nodeList.at(rerouteId).box;
			if (rerouteBox.Max.x > rightNodeBox.Min.x)
			{
				size_t firstLeftLinkId = extraGraph.GetFirstLinkId(rerouteId, {ELinkFlag::Left});
				auto& firstLeftLink = graph.linkList.at(firstLeftLinkId);
				if (link.leftPinId == firstLeftLink.leftPinId) continue;
				return CHANGE_LINK_LEFT_PIN(GET_LINK_TO_BREAK, link.id, firstLeftLink.leftPinId);
			}
		}

		size_t lastRerouteRightLinkId = extraGraph.GetFirstLinkId(it->second.back(), {ELinkFlag::Right});
		auto& lastRerouteRightLink = graph.linkList.at(lastRerouteRightLinkId);
		return CHANGE_LINK_LEFT_PIN(GET_LINK_TO_BREAK, link.id, lastRerouteRightLink.leftPinId);
	}

	static void createReroutesIfNeeded(size_t leftPinId,
		size_t linkToBreakId,
		EditableGraph& editableGraph,
		const std::list<size_t>& nodeIdList,
		std::map<size_t, std::list<size_t>>& leftPinIdToRerouteListMap)
	{
		auto& extraGraph = editableGraph.extraGraph;
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;

		auto& linkToBreak = graph.linkList.at(linkToBreakId);
		auto& extraLinkToBreak = extraGraph.extraLinkList.at(linkToBreakId);

		auto& leftNode = extraLinkToBreak.GetNode(graph, true);
		auto& leftNodeBox = leftNode.box;
		Box linkBox;
		bool isLinkAligned = NodePlacerCommon::GetLinkBox(linkBox, linkToBreak.id, editableGraph, leftNodeBox.Min.y, true);
		if (linkBox.getSize().x <= (extraLinkToBreak.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x))
			return; // link too short for reroute
		if (isLinkAligned)
		{
			if (isLineIntersect(linkToBreak.id, linkBox, editableGraph, nodeIdList)) // create 2 reroute nodes
				createReroutes(leftPinId, linkToBreak, linkBox, editableGraph, leftPinIdToRerouteListMap, ECreateReroute::Both);
			else // no reroute needed
			{
			}
		}
		else
		{
			auto& rightNodeBox = extraLinkToBreak.GetNode(graph, false).box;
			double leftAlignPosY = leftNodeBox.Min.y + extraLinkToBreak.GetPin(graph, true).offset.y
								   - CppGraphConfig::LINE_SCALE * graphConfig.lineWidth * 0.5;
			double rightAlignPosY = rightNodeBox.Min.y + extraLinkToBreak.GetPin(graph, false).offset.y
									+ CppGraphConfig::LINE_SCALE * graphConfig.lineWidth * 0.5;
			double newAlignPosY = leftNode.isRerouteNode ? leftAlignPosY : std::max(leftAlignPosY, rightAlignPosY);
			NodePlacerCommon::GetLinkBox(linkBox, linkToBreak.id, editableGraph, newAlignPosY, false, true);
			ECreateReroute createRerouteType
				= !leftNode.isRerouteNode && leftAlignPosY < rightAlignPosY ? ECreateReroute::Left : ECreateReroute::Right;
			if (createRerouteType == ECreateReroute::Left)
				linkBox.Min.x = leftNodeBox.Max.x + (extraLinkToBreak.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x);
			else
				linkBox.Max.x = rightNodeBox.Min.x - (extraLinkToBreak.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x);
			if (isLineIntersect(linkToBreak.id, linkBox, editableGraph, nodeIdList)) // create 2 reroute nodes
				createReroutes(leftPinId, linkToBreak, linkBox, editableGraph, leftPinIdToRerouteListMap, ECreateReroute::Both);
			else // create 1 reroute node
				createReroutes(leftPinId, linkToBreak, linkBox, editableGraph, leftPinIdToRerouteListMap, createRerouteType);
		}
	}

	// create one or 2 reroutes
	static void createReroutes(size_t leftPinId,
		const CppLink& link,
		const Box& linkBox,
		EditableGraph& editableGraph,
		std::map<size_t, std::list<size_t>>& leftPinIdToRerouteListMap,
		ECreateReroute createRerouteType)
	{
		auto& extraGraph = editableGraph.extraGraph;
		auto& graph = editableGraph.graph;

		auto& extraLink = extraGraph.extraLinkList.at(link.id);
		auto& rightNodeBox = extraLink.GetNode(graph, false).box;
		if (linkBox.Max.x == rightNodeBox.Min.x) // create 1 reroute node since no place for 2
		{
			auto& leftNodeBox = extraLink.GetNode(graph, true).box;
			double leftAlignPosY = leftNodeBox.Min.y + extraLink.GetPin(graph, true).offset.y;
			double rightAlignPosY = rightNodeBox.Min.y + extraLink.GetPin(graph, false).offset.y;
			createRerouteType = leftAlignPosY < rightAlignPosY ? ECreateReroute::Left : ECreateReroute::Right;
		}

		CREATE_REROUTE(CREATE_REROUTES, link.id, false);
		size_t leftRerouteNodeId = extraGraph.extraNodeList.size() - 1;
		MOVE_NODE(CREATE_REROUTES, leftRerouteNodeId, createRerouteType == ECreateReroute::Right ? linkBox.Max : linkBox.Min);
		auto it = leftPinIdToRerouteListMap.find(leftPinId);
		if (it == leftPinIdToRerouteListMap.end()) leftPinIdToRerouteListMap[leftPinId] = {leftRerouteNodeId};
		else
		{
			auto& leftNode = extraLink.GetNode(graph, true);
			auto leftNodeIt = std::find(it->second.begin(), it->second.end(), leftNode.id);
			if (leftNodeIt == it->second.end()) throw std::runtime_error("ReroutePlacerY::createReroutes: leftNode not found");
			leftNodeIt++;
			bool isLengthOne = (++it->second.begin()) == it->second.end();
			if ((isLengthOne || leftNodeIt != it->second.end()) && createRerouteType != ECreateReroute::Both)
			// if reroute inserted between 2 reroutes or insertion of 2nd reroute, change the link after left pin
			{
				size_t firstRightLinkId = extraGraph.GetFirstLinkId(leftNode.id, {ELinkFlag::Right});
				size_t firstRightRerouteLinkId = extraGraph.GetFirstLinkId(leftRerouteNodeId, {ELinkFlag::Right});
				auto& firstRightRerouteLink = graph.linkList.at(firstRightRerouteLinkId);
				CHANGE_LINK_LEFT_PIN(CREATE_REROUTES, firstRightLinkId, firstRightRerouteLink.leftPinId);
			}
			it->second.insert(leftNodeIt, leftRerouteNodeId);
		}

		if (createRerouteType != ECreateReroute::Both) return;

		size_t firstRightLinkId = extraGraph.GetFirstLinkId(leftRerouteNodeId, {ELinkFlag::Right});
		CREATE_REROUTE(CREATE_REROUTES, firstRightLinkId, false);
		size_t rightRerouteNodeId = extraGraph.extraNodeList.size() - 1;
		MOVE_NODE(CREATE_REROUTES, rightRerouteNodeId, linkBox.Max);

		it = leftPinIdToRerouteListMap.find(leftPinId);
		auto newNodeIt = std::find(it->second.begin(), it->second.end(), leftRerouteNodeId);
		newNodeIt++;
		it->second.insert(newNodeIt, rightRerouteNodeId);
	}
};
