// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <map>
#include <vector>

#include "../CppCommon.h"
#include "./CppComment.h"
#include "./CppGraphConfig.h"
#include "./CppLink.h"
#include "./CppNode.h"
#include "./CppPin.h"

struct CppGraph
{
	// ======== ATTRIBUTES ========

	CppGraphConfig graphConfig;

	std::map<size_t, CppNode> nodeList;
	std::map<size_t, CppComment> commentList;
	std::map<size_t, CppPin> pinList;
	std::map<size_t, CppLink> linkList;

	std::vector<size_t> selectedNodeIdList;
	std::vector<size_t> selectedCommentIdList;
};