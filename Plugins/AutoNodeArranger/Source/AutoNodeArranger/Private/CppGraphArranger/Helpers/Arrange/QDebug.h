// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <iostream>

class CustomCout
{
public:
	static CustomCout& instance()
	{
		static CustomCout instance;
		return instance;
	}

	friend CustomCout& operator<<(CustomCout& customCout, bool b)
	{
		std::cout << (b ? "true " : "false ");
		return customCout;
	}

	template <typename T> friend CustomCout& operator<<(CustomCout& customCout, const T& t)
	{
		std::cout << t << " ";
		return customCout;
	}

	friend CustomCout& operator<<(CustomCout& customCout, std::ostream& (*f)(std::ostream&))
	{
		std::cout << f;
		return customCout;
	}
};

#define QDEBUG(t) CustomCout::instance() << #t << ":" << t << std::endl
