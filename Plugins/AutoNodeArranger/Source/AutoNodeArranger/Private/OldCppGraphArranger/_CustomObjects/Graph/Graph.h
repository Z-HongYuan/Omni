// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <vector>

#include "../../../CppGraphArranger/DataTypes/CppCommon.h"
#include "../../../CppGraphArranger/DataTypes/Graph/CppGraphConfig.h"

template <typename PinType, typename NodeType, typename CommentType> struct OldCppGraph
{
	// ======== ATTRIBUTES ========

	CppGraphConfig graphConfig;

	std::vector<PinType> pinList;
	std::vector<NodeType> nodeList;
	std::vector<CommentType> commentList;

	std::vector<size_t> selectedNodeIDList;
	std::vector<size_t> selectedCommentIDList;

	// ======== CONSTRUCTOR ========

	OldCppGraph() {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, OldCppGraph& graph)
	{
		is >> graph.graphConfig;
		read_vector(is, graph.pinList);
		read_vector(is, graph.nodeList);
		read_vector(is, graph.commentList);
		read_vector(is, graph.selectedNodeIDList);
		read_vector(is, graph.selectedCommentIDList);
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const OldCppGraph& graph)
	{
		os << graph.graphConfig << "\n";
		write_vector(os, graph.pinList, "\n\t") << "\n";
		write_vector(os, graph.nodeList, "\n\t") << "\n";
		write_vector(os, graph.commentList, "\n\t") << "\n";
		write_vector(os, graph.selectedNodeIDList, " ") << "\n";
		write_vector(os, graph.selectedCommentIDList, " ");
		return os;
	}
};