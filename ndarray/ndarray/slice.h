#pragma once

#include <cstdint>

using namespace std;

class Ellipsis {};

template <size_t D = 0>
class Slice
{
	public:
		int64_t start;
		int64_t stop;
		uint64_t step;
		size_t dim;

		Slice() {}

		Slice(int64_t _start = 0, int64_t _stop = -1, uint64_t _step = -1) :
			start(_start),
			stop(_stop),
			step(_step),
			dim(D) {}

		void update(size_t length)
		{
			if (start < 0)
			{
				start += length;
			}

			if (stop < 0)
			{
				stop += length;
			}
		}
};