// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <chrono>
#include <limits>
#include <random>

static int64_t timeSeed() { return std::chrono::system_clock::now().time_since_epoch().count(); }

class RandomHelper
{
public:
	RandomHelper(int64_t seed_) : seed(seed_), m_randomEngine(seed_) {}

	int64_t randomInt(int64_t min = std::numeric_limits<int64_t>::min(), int64_t max = std::numeric_limits<int64_t>::max())
	{
		std::uniform_int_distribution<int64_t> distribution(min, max);
		return distribution(m_randomEngine);
	}

	double randomDouble(double min = 0., double max = 1.)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(m_randomEngine);
	}

	bool randomBool(double trueStrength = 0.5) { return randomDouble(0., 1.) <= trueStrength; }

	int64_t getSeed() const { return seed; }

private:
	int64_t seed;
	std::mt19937 m_randomEngine;
};
