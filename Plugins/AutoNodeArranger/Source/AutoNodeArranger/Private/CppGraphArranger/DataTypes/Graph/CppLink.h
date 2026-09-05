// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../Basic/Vector2.h"

struct CppLink
{
	// ======== ATTRIBUTES ========

	size_t id = -1;
	size_t leftPinId = -1;
	size_t rightPinId = -1;

	// ======== CONSTRUCTOR ========

	CppLink() = default;
	CppLink(size_t id_, size_t leftPinId_, size_t rightPinId_) : id(id_), leftPinId(leftPinId_), rightPinId(rightPinId_) {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, CppLink& cppLink)
	{
		return is >> cppLink.id >> cppLink.leftPinId >> cppLink.rightPinId;
	}

	friend std::ostream& operator<<(std::ostream& os, const CppLink& cppLink)
	{
		return os << cppLink.id << " " << cppLink.leftPinId << " " << cppLink.rightPinId;
	}
};
