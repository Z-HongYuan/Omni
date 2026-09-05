// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <iostream>

struct LinkInstruction
{
	// ======== ATTRIBUTES ========

	// fromPinId is pin on the left
	size_t fromPinId = static_cast<size_t>(-1);
	// toPinId is pin on the right
	size_t toPinId = static_cast<size_t>(-1);

	bool bBreak = false;

	// ======== CONSTRUCTOR ========

	LinkInstruction() = default;
	LinkInstruction(size_t fromPinId_, size_t toPinId_, bool bBreak_) : fromPinId(fromPinId_), toPinId(toPinId_), bBreak(bBreak_)
	{
	}

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, LinkInstruction& linkInstruction)
	{
		return is >> linkInstruction.fromPinId >> linkInstruction.toPinId >> linkInstruction.bBreak;
	}

	friend std::ostream& operator<<(std::ostream& os, const LinkInstruction& linkInstruction)
	{
		return os << linkInstruction.fromPinId << " " << linkInstruction.toPinId << " " << linkInstruction.bBreak;
	}
};
