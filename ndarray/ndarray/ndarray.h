#pragma once

#include <iostream>
#include <array>
#include <functional>
#include <algorithm>
#include <memory>
#include <math.h>

#include "utils.h"
#include "funcs.h"
#include "slice.h"
#include "iterator.h"
#include "exceptions.h"

using namespace std;

Ellipsis EL = Ellipsis();
Ellipsis EE = Ellipsis();

template <typename T, size_t N> class Ndarray;
template <typename T, size_t N> class NdarrayBase;

template <typename T, size_t N> 
class NdarrayBase
{
	public:
		typedef DataIterator<T, N> iterator;
		typedef DataIterator<const T, N> const_iterator;

		size_t size;
		size_t offset;
		shared_ptr<T> data;
		array<size_t, N> shape;
		array<size_t, N> strides;

		NdarrayBase(size_t _size, size_t _offset, shared_ptr<T> _data, array<size_t, N> _shape, array<size_t, N> _strides) :
			size(_size),
			offset(_offset),
			data(_data),
			shape(_shape),
			strides(_strides) {}

		NdarrayBase() :
			size(0),
			offset(0),
			data(shared_ptr<T>(nullptr)),
			shape(array<size_t, N>()),
			strides(array<size_t, N>()) {}

		NdarrayBase(const NdarrayBase<T, N>& other) :
			// Copy constructor
			size(other.size),
			offset(other.offset),
			data(other.data),
			shape(other.shape),
			strides(other.strides) {}

		NdarrayBase(NdarrayBase<T, N>&& other) :
			// Move constructor
			NdarrayBase<T, N>()
		{
			swap(*this, other);
		}

		iterator begin() const { return iterator(&data.get()[0], shape, strides); }

		iterator end() const
		{
			auto end = shape[0] * filterZeros(strides)[0];
			return iterator(&data.get()[end], shape, strides);
		}

		const_iterator cbegin() const { return iterator(&data.get()[0], shape, strides); }

		const_iterator cend() const
		{
			auto end = shape[0] * filterZeros(strides)[0];
			return iterator(&data.get()[end], shape, strides);
		}

		/*NdarrayBase<T, N>& operator=(NdarrayBase<T, N> that)
		{
			// Copy assignment operator
			swap(*this, that);
			return *this;
		}*/

		/*friend void swap(NdarrayBase<T, N>& first, NdarrayBase<T, N>& second)
		{
			swap(first.size, second.size);
			swap(first.offset, second.offset);
			swap(first.shape, second.shape);
			swap(first.data, second.data);
			swap(first.strides, second.strides);
		}*/

		NdarrayBase<T, N>& operator=(const T& other)
		{
			std::fill(this->begin(), this->end(), other);
			return *this;
		}

		NdarrayBase<T, N>& operator=(const NdarrayBase<T, N>& other)
		{
			auto tmp = other.broadcastTo(shape);
			std::copy(tmp.begin(), tmp.end(), this->begin());
			return (*this);
		}

		template <size_t M>
		NdarrayBase<T, N>& operator=(const NdarrayBase<T, M>& other)
		{
			auto tmp = other.broadcastTo(shape);
			std::copy(tmp.begin(), tmp.end(), this->begin());
			return (*this);
		}

		void checkIndex(const int64_t idx, const uint64_t dim) const
		{
			if (N < 1)
				throw IndexError("Too many indices!");

			if (dim > (N - 1))
				throw IndexError("Invalid index dimension!");

			if ((idx < 0) or (idx >= static_cast<int64_t>(shape[dim])))
				throw IndexError("Index out of bounds!");
		}

		template <size_t DIM>
		void checkIndex(const Slice<DIM> slc) const
		{
			checkIndex(slc.start, DIM);

			// stop can be larger than the dimension length
			//checkIndex(slc.stop, DIM);
		}

		size_t updateIndex(int64_t idx, const size_t dim) const
		{
			if (idx < 0)
			{
				idx += this->shape[dim];
			}

			return idx;
		}

		template <size_t M>
		Ndarray<T, M> broadcastTo(const array<size_t, M> newShape) const
		{
			static_assert(N <= M, "Can only broadcast to equal or higher dimensionality!");

			auto newStrides = filledArray<size_t, M>(0);
			auto offset = newShape.size() - shape.size();

			for (int64_t i = shape.size() - 1; i >= 0; i--)
			{
				if (newShape[i + offset] == shape[i])
				{
					newStrides[i + offset] = strides[i];
				}
				else if (shape[i] != 1)
					throw DimensionError("Operands could not be broadcast together!");
			}

			return Ndarray<T, M>(product(newShape), this->offset, this->data, newShape, newStrides);
		}
};

template <typename T, size_t N>
class Ndarray : public NdarrayBase<T, N>
{
	public:
		using NdarrayBase<T, N>::NdarrayBase;
		using NdarrayBase<T, N>::operator=;

		Ndarray<T, N> operator[](const Ellipsis) { return Ndarray<T, N>(*this); }

		template <size_t DIM>
		Ndarray<T, N> operator[](Slice<DIM> slc)
		{
			static_assert(DIM < N, "Invalid index dimension!");

			slc.update(this->shape[DIM]);
			this->checkIndex(slc);

			auto newShape = this->shape;
			auto newStrides = this->strides;
			auto start = slc.start * newStrides[DIM];

			newShape[DIM] = ceil((slc.stop - slc.start) / static_cast<double>(slc.step));
			newStrides[DIM] = newStrides[DIM] * slc.step;

			return Ndarray<T, N>(product(newShape), start + this->offset, shared_ptr<T>(this->data, (this->data).get() + start), newShape, newStrides);
		}

		template <size_t DIM>
		const Ndarray<T, N> operator[](const Slice<DIM> slc) const { return operator[](slc); }

		Ndarray<T, N - 1> operator[](int64_t idx)
		{
			idx = this->updateIndex(idx, 0);
			this->checkIndex(idx, 0);

			auto newShape = subarray<size_t, N - 1>(this->shape, 1);
			auto start = idx * this->strides[0];

			return Ndarray<T, N - 1>(product(newShape), start + this->offset, shared_ptr<T>(this->data, this->data.get() + start), newShape, subarray<size_t, N - 1>(this->strides, 1));
		}

		template <size_t DIM>
		const Ndarray<T, N> operator[](const int64_t slc) const { return operator[](slc); }
};

template <typename T>
class Ndarray<T, 1> : public NdarrayBase<T, 1>
{
	public:
		using NdarrayBase<T, 1>::NdarrayBase;
		using NdarrayBase<T, 1>::operator=;

		Ndarray<T, 1> operator[](const Ellipsis) { return Ndarray<T, 1>(*this); }

		T& operator[](int64_t idx)
		{
			idx = this->updateIndex(idx, 0);
			this->checkIndex(idx, 0);
			auto start = idx * this->strides[0];
			return this->data.get()[start];
		}

		const Ndarray<T, 1> operator[](const int64_t idx) const { return operator[](idx); }

		Ndarray<T, 1> operator[](Slice<0> slc)
		{
			slc.update(this->shape[0]);
			this->checkIndex(slc);

			auto newShape = this->shape;
			auto newStrides = this->strides;
			auto start = slc.start * newStrides[0];

			newShape[0] = ceil((slc.stop - slc.start) / static_cast<double>(slc.step));
			newStrides[0] = newStrides[0] * slc.step;

			return Ndarray<T, 1>(1, start + this->offset, shared_ptr<T>(this->data, (this->data).get() + start), newShape, newStrides);
		}

		const Ndarray<T, 1> operator[](Slice<0> slc) const { return operator[](slc); }
};