// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <set>

struct ExtraComment
{
	// ======== ATTRIBUTES ========

	size_t id = -1; // not required, but make code easier to write
	std::set<size_t> insideNodeIdSet;
	std::set<size_t> insideCommentIdSet;
	// nodes that are to the left and to the right of this comment
	std::set<size_t> leftRightNodeIdSet;
	// comments that contain at least one node to the left and to the right of this comment
	std::set<size_t> leftRightCommentIdSet;
	// nodes that are outside the comment and enter this comment to the first right link
	std::set<size_t> leftEdgeOutsideFirstRightNodeIdSet;
	// nodes that are outside the comment and exit this comment to the first right link
	std::set<size_t> rightEdgeOutsideFirstRightNodeIdSet;
};
