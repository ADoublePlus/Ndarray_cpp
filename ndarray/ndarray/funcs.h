#pragma once

#include <type_traits>
#include <vector>
#include <functional>

#include "utils.h"

using namespace std;

template <typename T, size_t N>
class Ndarray;

template <typename T, typename = typename enable_if<is_arithmetic<T>::value, T>::type>
vector<T> calcStrides(const vector<T> arg) // Returns standard strides for a given shape
{
	auto out = vector<T>(arg.size(), 1);

	for (int64_t i = arg.size() - 2; i >= 0; i--)
	{
		out[i] = arg[i + 1] * out[i + 1];
	}

	return out;
}

template <typename T, size_t N, typename = typename enable_if<is_arithmetic<T>::value, T>::type>
array<T, N> calcStrides(const array<T, N> arg)
{
	array<T, N> out;
	out.back() = 1;

	for (int64_t i = arg.size() - 2; i >= 0; i--)
	{
		out[i] = arg[i + 1] * out[i + 1];
	}

	return out;
}

template <typename T, size_t N>
Ndarray<T, N> ndarray(array<size_t, N> _shape, function<void(T*)> _destructor = _deleteArray<T>)
{
	auto size = product(_shape);
	return Ndarray<T, N>(size, 0, shared_ptr<T>(new T[size], _destructor), _shape, calcStrides(_shape));
}

template <typename T, size_t N>
Ndarray<T, N> ndarray(T* _data, array<size_t, N> _shape, function<void(T*)> _destructor = _deleteNothing<T>)
{
	return Ndarray<T, N>(product(_shape), 0, shared_ptr<T>(_data, _destructor), _shape, calcStrides(_shape));
}

template <typename T, size_t N>
Ndarray<T, N> ndarray(T* _data, array<size_t, N> _shape, array<size_t, N> _strides, function<void(T*)> _destructor = _deleteNothing<T>)
{
	return Ndarray<T, N>(product(_shape), 0, shared_ptr<T>(_data, _destructor), _shape, _strides);
}