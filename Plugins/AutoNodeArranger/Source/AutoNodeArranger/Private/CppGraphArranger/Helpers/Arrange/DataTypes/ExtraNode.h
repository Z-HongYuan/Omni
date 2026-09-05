// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <array>
#include <list>
#include <set>
#include <vector>

#include "../../../DataTypes/Graph/CppLink.h"

enum ELinkFlag
{
	None = 0,
	LeftExec = 1 << 0,
	LeftNonExec = 1 << 1,
	RightExec = 1 << 2,
	RightNonExec = 1 << 3,
	Left = LeftExec | LeftNonExec,
	Right = RightExec | RightNonExec,
	Exec = LeftExec | RightExec,
	NonExec = LeftNonExec | RightNonExec,
	All = Left | Right
};

#include "../QDebug.h"

struct ExtraNode
{
	// ======== ATTRIBUTES ========

	bool isOneOutputRerouteNode = false;
	std::array<std::array<std::list<size_t>, 2>, 2> linkIdListArrayArray;
	std::set<size_t> insideCommentIdSet;	// comments containing this node
	std::set<size_t> leftRightCommentIdSet; // comments where this node is to the left and to the right

	// a neighbour represents a node which is in the same comment as the current one
	std::set<size_t> neighbourLeftRightCommentIdSet; // comments where at least one neighbour is to the left and to the right

	std::set<size_t> leftEdgeOutsideCommentIdSet; // comment not containing this node just to the left of this node

	std::set<size_t> rightEdgeOutsideCommentIdSet; // comment not containing this node just to the right of this node

	bool isDeleted = false;
	std::set<size_t> loopChildIdSet;
	size_t loopBrotherId = static_cast<size_t>(-1);
	bool isLoopBrotherPlacedOnRight = false;

	// ======== METHODS ========

	std::vector<std::list<size_t>*> GetLinkIdListList(const std::initializer_list<ELinkFlag>& linkFlags)
	{
		std::vector<std::list<size_t>*> linkListList;
		for (auto linkFlag : linkFlags)
		{
			if (linkFlag & ELinkFlag::LeftExec) linkListList.push_back(&linkIdListArrayArray[0][0]);
			if (linkFlag & ELinkFlag::LeftNonExec) linkListList.push_back(&linkIdListArrayArray[0][1]);
			if (linkFlag & ELinkFlag::RightExec) linkListList.push_back(&linkIdListArrayArray[1][0]);
			if (linkFlag & ELinkFlag::RightNonExec) linkListList.push_back(&linkIdListArrayArray[1][1]);
		}
		return linkListList;
	}

	std::vector<std::list<size_t>*> GetLinkIdListList(const std::initializer_list<ELinkFlag>& linkFlags) const
	{
		return const_cast<ExtraNode&>(*this).GetLinkIdListList(linkFlags);
	}

	size_t GetCount(const std::initializer_list<ELinkFlag>& linkFlags) const
	{
		size_t count = 0;
		for (auto linkList : GetLinkIdListList(linkFlags)) count += linkList->size();
		return count;
	}

	std::list<size_t>& GetLinkIdList(bool bLeft, bool bExec) { return linkIdListArrayArray[bLeft ? 0 : 1][bExec ? 0 : 1]; }

	const std::list<size_t>& GetLinkIdList(bool bLeft, bool bExec) const
	{
		return const_cast<ExtraNode&>(*this).GetLinkIdList(bLeft, bExec);
	}
};
