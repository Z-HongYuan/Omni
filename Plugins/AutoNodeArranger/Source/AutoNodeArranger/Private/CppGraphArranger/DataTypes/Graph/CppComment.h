// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "../Basic/Box.h"
#include "../Basic/Vector2.h"
#include "../CppCommon.h"

struct CppComment
{
	// ======== ATTRIBUTES ========

	size_t id = -1;
	std::string name;

	Box box;

	// ======== CONSTRUCTOR ========

	CppComment() = default;
	CppComment(size_t id_, const std::string& name_, const Box& box_) : id(id_), name(name_), box(box_)
	{
		for (auto c : " \n") name.erase(std::remove(name.begin(), name.end(), c), name.end());
		if (name.empty()) name = std::string("_") + std::to_string(id);
	}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, CppComment& comment)
	{
		is >> comment.id;
		read_string(is, comment.name);
		is >> comment.box;
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const CppComment& comment)
	{
		os << comment.id << " ";
		write_string(os, comment.name);
		os << "\n\t" << comment.box;
		return os;
	}
};