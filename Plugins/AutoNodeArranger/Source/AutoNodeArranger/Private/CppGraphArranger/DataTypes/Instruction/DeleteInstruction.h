// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <iostream>

struct DeleteInstruction
{
	// ======== ATTRIBUTES ========

	size_t nodeId = -1;
	bool bCancel = false;

	// ======== CONSTRUCTOR ========

	DeleteInstruction() = default;
	DeleteInstruction(size_t nodeId_, bool bCancel_) : nodeId(nodeId_), bCancel(bCancel_) {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, DeleteInstruction& deleteInstruction)
	{
		return is >> deleteInstruction.nodeId >> deleteInstruction.bCancel;
	}

	friend std::ostream& operator<<(std::ostream& os, const DeleteInstruction& deleteInstruction)
	{
		return os << deleteInstruction.nodeId << ' ' << deleteInstruction.bCancel;
	}
};
