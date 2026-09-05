// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../Basic/Vector2.h"

struct NodeInstruction
{
	// ======== ATTRIBUTES ========

	size_t nodeId = -1;

	Vector2 fromPos;
	Vector2 toPos;

	// ======== CONSTRUCTOR ========

	NodeInstruction() = default;
	NodeInstruction(size_t nodeId_, Vector2 fromPos_, Vector2 toPos_) : nodeId(nodeId_), fromPos(fromPos_), toPos(toPos_) {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, NodeInstruction& nodeInstruction)
	{
		return is >> nodeInstruction.nodeId >> nodeInstruction.fromPos >> nodeInstruction.toPos;
	}

	friend std::ostream& operator<<(std::ostream& os, const NodeInstruction& nodeInstruction)
	{
		return os << nodeInstruction.nodeId << " " << nodeInstruction.fromPos << " " << nodeInstruction.toPos;
	}
};
