// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <string>

#include "../Basic/Vector2.h"
#include "../CppCommon.h"

static const Vector2 REROUTE_PIN_OFFSET(10, 0);

struct CppPin
{
private:
	static inline size_t emptyIdCount = 0;

public:
	// ======== ATTRIBUTES ========

	size_t id = -1;
	size_t ownerNodeId = -1;
	std::string name;
	bool isExec = false;
	bool isLeft = false;

	Vector2 offset;

	// ======== CONSTRUCTOR ========

	CppPin() = default;
	CppPin(size_t id_, size_t ownerNodeId_, const std::string& name_, bool isExec_, bool isLeft_, const Vector2& offset_) :
		id(id_), ownerNodeId(ownerNodeId_), name(name_), isExec(isExec_), isLeft(isLeft_), offset(offset_)
	{
		for (auto c : " \n") name.erase(std::remove(name.begin(), name.end(), c), name.end());
		if (name.empty()) name = std::string("_") + std::to_string(++emptyIdCount);
	}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, CppPin& pin)
	{
		is >> pin.id >> pin.ownerNodeId;
		read_string(is, pin.name);
		is >> pin.isExec >> pin.isLeft >> pin.offset;
		return is;
	}

	friend std::ostream& operator<<(std::ostream& os, const CppPin& pin)
	{
		os << pin.id << " " << pin.ownerNodeId << " ";
		write_string(os, pin.name);
		os << " " << pin.isExec << " " << pin.isLeft << " " << pin.offset;
		return os;
	}
};
