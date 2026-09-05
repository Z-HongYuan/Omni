// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "../Basic/Box.h"
#include "../CppCommon.h"

struct CppNode
{
	// ======== ATTRIBUTES ========

	size_t id = -1;
	std::string name;
	Box box;
	bool isRerouteNode = false;

	// ======== CONSTRUCTOR ========

	CppNode() = default;
	CppNode(size_t id_, const std::string& name_, const Box& box_, bool isRerouteNode_) :
		id(id_), name(name_), box(box_), isRerouteNode(isRerouteNode_)
	{
		for (auto c : " \n") name.erase(std::remove(name.begin(), name.end(), c), name.end());
		if (name.empty()) name = std::string("_") + std::to_string(id);
	}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, CppNode& node)
	{
		is >> node.id;
		read_string(is, node.name);
		is >> node.box >> node.isRerouteNode;
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const CppNode& node)
	{
		os << node.id << " ";
		write_string(os, node.name);
		return os << "\n\t" << node.box << "\n\t" << node.isRerouteNode << "\n";
	}
};