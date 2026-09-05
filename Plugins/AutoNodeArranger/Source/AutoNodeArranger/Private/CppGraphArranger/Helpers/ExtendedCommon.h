// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <set>
#include <vector>

template <typename T> bool contains(const std::set<T>& tSet, const T& toFindT) { return tSet.count(toFindT) > 0; }

template <typename U, typename T> bool contains(const U& tList, const T& toFindT)
{
	auto itEnd = std::end(tList);
	return std::find(std::begin(tList), itEnd, toFindT) != itEnd;
}

template <typename U, typename T> bool contains_if(const U& tList, const T& tFinder)
{
	auto itEnd = std::end(tList);
	return std::find_if(std::begin(tList), itEnd, tFinder) != itEnd;
}

// first template type must be specified explicitly
template <typename V, typename U, typename T> std::vector<V> copy_if_vector(const U& tList, const T& tFinder)
{
	std::vector<V> result;
	auto itEnd = std::end(tList);
	std::copy_if(std::begin(tList), itEnd, std::back_inserter(result), tFinder);
	return result;
}

// first template type must be specified explicitly
template <typename V, typename U, typename T> std::set<V> copy_if_set(const U& tList, const T& tFinder)
{
	std::set<V> result;
	auto itEnd = std::end(tList);
	std::copy_if(std::begin(tList), itEnd, std::inserter(result, result.end()), tFinder);
	return result;
}

template <typename T> std::set<T> difference(const std::set<T>& lhsSet, const std::set<T>& rhsSet)
{
	std::set<T> result;
	std::set_difference(lhsSet.begin(), lhsSet.end(), rhsSet.begin(), rhsSet.end(), std::inserter(result, result.begin()));
	return result;
}

template <typename T> std::set<T> intersection(const std::set<T>& lhsSet, const std::set<T>& rhsSet)
{
	std::set<T> result;
	std::set_intersection(lhsSet.begin(), lhsSet.end(), rhsSet.begin(), rhsSet.end(), std::inserter(result, result.begin()));
	return result;
}

template <typename T> bool findIndex(const std::vector<T>& vector, const T& value, size_t& index)
{
	auto it = std::find(vector.begin(), vector.end(), value);
	if (it == vector.end()) return false;
	index = std::distance(vector.begin(), it);
	return true;
}