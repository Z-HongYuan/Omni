// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../../DataTypes/Basic/Vector2.h"
#include "../../../DataTypes/Graph/CppGraph.h"

struct ExtraLink
{
	// ======== ATTRIBUTES ========

	size_t id = -1; // not required, but make code easier to write
	bool isExec = false;
	double pinDiffY = 0.0;	// left to right
	double nodeDiffX = 0.0; // left to right
	bool bFinishWithRerouteOneOutput = false;
	bool bBetweenTwoRerouteOneOutput = false;
	bool isBroken = false;

	// ======== METHODS ========

	const auto& GetNode(const CppGraph& graph, bool bLeft) const
	{
		auto& link = graph.linkList.at(id);
		return graph.nodeList.at(graph.pinList.at(bLeft ? link.leftPinId : link.rightPinId).ownerNodeId);
	}

	const auto& GetPin(const CppGraph& graph, bool bLeft) const
	{
		auto& link = graph.linkList.at(id);
		return graph.pinList.at(bLeft ? link.leftPinId : link.rightPinId);
	}

	const auto& GetNodeConnectedTo(const CppGraph& graph, size_t fromNodeId) const
	{
		auto& link = graph.linkList.at(id);
		auto leftNodeId = graph.pinList.at(link.leftPinId).ownerNodeId;
		auto rightNodeId = graph.pinList.at(link.rightPinId).ownerNodeId;
		return graph.nodeList.at(leftNodeId == fromNodeId ? rightNodeId : leftNodeId);
	}
};
