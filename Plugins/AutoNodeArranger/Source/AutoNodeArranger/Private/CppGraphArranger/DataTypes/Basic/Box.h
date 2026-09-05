// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "Vector2.h"

static const Vector2 COMMENT_HEADER(0., 30.);

struct Box
{
	// ======== ATTRIBUTES ========

	Vector2 Min;
	Vector2 Max;

	// ======== CONSTRUCTOR ========

	Box() = default;
	Box(double minX, double minY, double maxX, double maxY) : Box(Vector2(minX, minY), Vector2(maxX, maxY)) {}
	Box(Vector2 Min_, Vector2 Max_) : Min(Min_), Max(Max_) {}

	static Box PosSize(Vector2 pos, Vector2 size) { return Box(pos, pos + size); }

	// ======== READ/WRITE ========

	friend std::istream& operator>>(std::istream& is, Box& box) { return is >> box.Min >> box.Max; }

	friend std::ostream& operator<<(std::ostream& os, const Box& box) { return os << box.Min << " " << box.Max; }

	// ======== METHODS ========

	Vector2 getSize() const { return Max - Min; }

	bool intersectWithInX(const Box& rhs) const
	{
		if (Min.x > rhs.Max.x || rhs.Min.x > Max.x) return false;
		return true;
	}

	bool intersectWith(const Box& rhs) const
	{
		if (Min.x > rhs.Max.x || rhs.Min.x > Max.x) return false;
		if (Min.y > rhs.Max.y || rhs.Min.y > Max.y) return false;
		return true;
	}

	Box extendBox(Vector2 extend) const { return Box(Min - extend, Max + extend); }

	Box withCommentHeader() const { return Box(Min - COMMENT_HEADER, Max); }

	Box offsetBox(Vector2 offset) const { return Box(Min + offset, Max + offset); }

	// used by connectedGraph
	Box computeBoxToNotOverlapInY(
		int commentHeaderCount, int commentDiffCount, const Vector2& spacing, const Vector2& commentSpacing) const
	{
		Box result = *this;
		result.Max += COMMENT_HEADER * commentHeaderCount;
		return result.extendBox(spacing + commentSpacing * commentDiffCount);
	}

	// used by connectedGraph
	Box computeBoxToNotOverlapInX(
		int commentHeaderCount, int commentDiffCount, const Vector2& spacing, const Vector2& commentSpacing) const
	{
		Box result = *this;
		result.Min += COMMENT_HEADER * commentHeaderCount;
		return result.extendBox(spacing + commentSpacing * commentDiffCount);
	}

	std::string toString() const { return "(" + Min.toString() + " : " + Max.toString() + ")"; }

	// ======== OPERATORS ========

	Box& operator+=(const Box& rhs)
	{
		Min = Min.minWith(rhs.Min);
		Max = Max.maxWith(rhs.Max);
		return *this;
	}
};
