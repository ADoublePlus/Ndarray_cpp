#pragma once

#include <vector>

#include "utils.h"

using namespace std;

template <typename T, size_t N>
class DataIterator : public iterator<forward_iterator_tag, T, ptrdiff_t, T*, T&>
{
	public:
		DataIterator(T* _buffer, const array<size_t, N>& _shape, const array<size_t, N>& _strides) : 
			buffer(_buffer), pos(&_buffer[0]), iter(0), shape(_shape), strides(_strides) {}

		bool operator==(const DataIterator<T, N>& iter) const { return (pos == iter.pos); }
		bool operator!=(const DataIterator<T, N>& iter) const { return (pos != iter.pos); }

		DataIterator<T, N>& operator++()
		{
			increment();
			return (*this);
		}

		DataIterator<T, N> operator++(const int)
		{
			auto out(*this);
			increment();
			return out;
		}

		T& operator*() { return *pos; }
		T* operator->() { return pos; }

		const T& operator*() const { return *pos; }
		const T* operator->() const { return pos; }

	private:
		T* buffer;
		T* pos;
		size_t iter;
		const array<size_t, N>& shape;
		const array<size_t, N>& strides;

		void increment()
		{
			size_t inc = 0;
			size_t tmp = iter++;

			for (int64_t i = shape.size() - 1; i >= 0; i--)
			{
				auto n = tmp % (shape[i]);
				inc += (n * strides[i]);
				tmp /= shape[i];
			}

			if (tmp > 0)
			{
				inc = shape[0] * filterZeros(strides)[0];
			}

			pos = buffer + inc;
		}
};