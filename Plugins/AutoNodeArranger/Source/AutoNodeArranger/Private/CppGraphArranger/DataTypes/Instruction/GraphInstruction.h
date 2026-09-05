// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <map>

#include "CommentInstruction.h"
#include "DeleteInstruction.h"
#include "LinkInstruction.h"
#include "NodeInstruction.h"
#include "RerouteInstruction.h"

struct GraphInstruction
{
	// ======== ATTRIBUTES ========

	std::vector<RerouteInstruction> rerouteInstructionList;
	std::vector<LinkInstruction> linkInstructionList;
	std::vector<CommentInstruction> commentInstructionList;
	std::vector<DeleteInstruction> deleteInstructionList;
	std::vector<NodeInstruction> allNodeInstructionList;

	std::vector<NodeInstruction> mergedNodeInstructionList;

	// ======== CONSTRUCTOR ========

	GraphInstruction() = default;

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, GraphInstruction& graphInstruction)
	{
		read_vector(is, graphInstruction.rerouteInstructionList);
		read_vector(is, graphInstruction.linkInstructionList);
		read_vector(is, graphInstruction.commentInstructionList);
		read_vector(is, graphInstruction.allNodeInstructionList);
		read_vector(is, graphInstruction.mergedNodeInstructionList);
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const GraphInstruction& graphInstruction)
	{
		write_vector(os, graphInstruction.rerouteInstructionList, " ") << "\n";
		write_vector(os, graphInstruction.linkInstructionList, "\n\t") << "\n";
		write_vector(os, graphInstruction.commentInstructionList, "\n\t") << "\n";
		write_vector(os, graphInstruction.allNodeInstructionList, "\n\t") << "\n";
		write_vector(os, graphInstruction.mergedNodeInstructionList, "\n\t");
		return os;
	}

	// ======== METHODS ========

	void updateMergedNodeInstructionList()
	{
		mergedNodeInstructionList.clear();
		std::map<int, std::vector<const NodeInstruction*>> nodeInstructionListMap; // key is NodeID
		for (const auto& nodeInstruction : allNodeInstructionList)
			nodeInstructionListMap[nodeInstruction.nodeId].push_back(&nodeInstruction);
		for (const auto& [nodeId, nodeInstructionList] : nodeInstructionListMap)
			mergedNodeInstructionList.emplace_back(
				nodeId, nodeInstructionList.front()->fromPos, nodeInstructionList.back()->toPos);
	}
};
