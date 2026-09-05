// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../_CustomObjects/Graph/Pin.h"

struct ExtendedPin : OldCppPin
{
	using OldCppPin::OldCppPin;

	// ======== ATTRIBUTES ========

	int owningNodeID = 0;

	// ======== METHODS ========

	void computePinExtension(int _owningNodeID) { owningNodeID = _owningNodeID; }
};
