#include <iostream>
#include <iterator>
#include <gtest/gtest.h>
#include <type_traits>

// -*- C++ -*-
// Copyright (c) 2017, Just Software Solutions Ltd
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted provided that the
// following conditions are met:
//
// 1. Redistributions of source code must retain the above
// copyright notice, this list of conditions and the following
// disclaimer.
//
// 2. Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials
// provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of
// its contributors may be used to endorse or promote products
// derived from this software without specific prior written
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// https://www.justsoftwaresolutions.co.uk/cplusplus/generating_sequences.html

namespace {

template <typename T>
class numeric_range
{
public:

	class iterator
	{
	public:

		class postinc_return 
		{
			T value_;

		public:

			postinc_return(T value) : value_(std::move(value)) {}
			T operator*()
			{
				return value_;
			}
		};

		void check_done()
		{
			if (range_->at_end())
				range_ = nullptr;
		}

		using value_type = int;
		using difference_type = std::ptrdiff_t;
		using pointer = int*;
		using reference = int&;
		using iterator_category = std::input_iterator_tag;

		explicit iterator(numeric_range* range) : range_(range) {}
		value_type operator*() const { return range_->current_; }
		value_type* operator->() { return &range_->current_; }

		iterator& operator++()
		{
			if (!range_)
				throw std::runtime_error("Increment a past- the-end iterator");
			range_->current_++;
			check_done();
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp(*this);
			++*this;
			return temp;
		}

		friend bool operator==(const iterator& lhs, const iterator& rhs)
		{
			return lhs.range_ == rhs.range_;
		}

		friend bool operator!=(const iterator& lhs, const iterator& rhs)
		{
			return !(lhs == rhs);
		}

	private:

		numeric_range* range_;
	};

	numeric_range(T current, T end)
		: current_(std::move(current))
		, end_(std::move(end))
	{
	}

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(nullptr); }

private:

	bool at_end()
	{
		return current_ >= end_; 
	}

	T current_;
	T end_;
};

template <typename T>
numeric_range<T> range(T to)
{
	return numeric_range<T>(T(), std::move(to));
}

template <typename T>
numeric_range<T> range(T from, T to)
{
	if (to <= from)
		throw std::runtime_error("Cannot count down");
	return numeric_range<T>(std::move(from), std::move(to));
}

}

TEST(PostInc, PostInc)
{
	range(10);
	const int limit = 10;
	numeric_range<int>(0, 10);
	numeric_range<int>(0, limit);
	for (auto x : range(limit))
		std::cout << x << ",";
	std::cout << std::endl;
	auto r = range(0, limit);
	std::vector<int> list(std::begin(r), std::end(r));
	EXPECT_EQ(list.back(), 9);
	// Can't handle
	// EXPECT_EQ(*std::begin(r), 0);
}
