// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <iostream>
#include <vector>

#include "../CppCommon.h"

struct CommentInstruction
{
	// ======== ATTRIBUTES ========

	size_t commentId = -1;

	std::vector<size_t> nodeIdUnderCommentList;
	std::vector<size_t> commentIdUnderCommentList;

	// ======== CONSTRUCTOR ========

	CommentInstruction() = default;
	explicit CommentInstruction(size_t commentId_) : commentId(commentId_) {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, CommentInstruction& commentInstruction)
	{
		return read_vector(read_vector(is >> commentInstruction.commentId, commentInstruction.nodeIdUnderCommentList),
			commentInstruction.commentIdUnderCommentList);
	}

	friend std::ostream& operator<<(std::ostream& os, const CommentInstruction& commentInstruction)
	{
		return write_vector(
			write_vector(os << commentInstruction.commentId << "\n\t", commentInstruction.nodeIdUnderCommentList, " ") << "\n\t",
			commentInstruction.commentIdUnderCommentList,
			" ");
	}
};
