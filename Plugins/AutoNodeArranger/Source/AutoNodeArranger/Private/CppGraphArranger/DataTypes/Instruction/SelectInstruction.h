// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../CppCommon.h"

struct SelectInstruction
{
	// ======== ATTRIBUTES ========

	std::set<size_t> nodeIdSet;
	std::set<size_t> commentIdSet;

	// ======== CONSTRUCTOR ========

	SelectInstruction() = default;
	SelectInstruction(const std::set<size_t>& nodeIdSet_, const std::set<size_t>& commentIdSet_) :
		nodeIdSet(nodeIdSet_), commentIdSet(commentIdSet_)
	{
	}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, SelectInstruction& selectNodeInstruction)
	{
		read_set(is, selectNodeInstruction.nodeIdSet);
		read_set(is, selectNodeInstruction.commentIdSet);
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const SelectInstruction& selectNodeInstruction)
	{
		write_set(os, selectNodeInstruction.nodeIdSet, " ") << "\n";
		write_set(os, selectNodeInstruction.commentIdSet, " ");
		return os;
	}
};
