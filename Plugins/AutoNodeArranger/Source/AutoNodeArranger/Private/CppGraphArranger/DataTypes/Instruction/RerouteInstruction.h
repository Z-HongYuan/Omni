// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <iostream>

#include "../Basic/Vector2.h"

struct RerouteInstruction
{
	// ======== ATTRIBUTES ========

	Vector2 pos;

	// ======== CONSTRUCTOR ========

	RerouteInstruction() = default;
	explicit RerouteInstruction(Vector2 pos_) : pos(pos_) {}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, RerouteInstruction& rerouteInstruction)
	{
		return is >> rerouteInstruction.pos;
	}

	friend std::ostream& operator<<(std::ostream& os, const RerouteInstruction& rerouteInstruction)
	{
		return os << rerouteInstruction.pos;
	}
};
