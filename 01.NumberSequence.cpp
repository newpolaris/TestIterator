#include <iostream>
#include <iterator>
#include <vector>
#include <gtest/gtest.h>

// https://www.artificialworlds.net/blog/2017/05/11/c-iterator-example-and-an-iterable-range/

class myit
{
private:

	int value_;
	class intholder
	{

		int value_;

	public:

		intholder(int value) : value_(value) {}
		int operator*() { return value_; }
	};

public:

	// Previously provided by std::iterator (deprecated since c++17)
	using value_type = int;
	using difference_type = std::ptrdiff_t;
	using pointer = int*;
	using reference = int&;
	using iterator_category = std::input_iterator_tag;

	explicit myit(int value) : value_(value) {}
	int operator*() const { return value_; }
	bool operator==(const myit& other) const { return value_ == other.value_; }
	bool operator!=(const myit& other) const { return !(*this == other); }

	intholder operator++(int)
	{
		intholder ret(value_);
		++*this;
		return ret;
	}

	myit& operator++()
	{
		++value_;
		return *this;
	}
};

class Numbers
{
private:

	const int start_;
	const int end_;

public:

	Numbers(int start, int end) : start_(start), end_(end) {}
	myit begin() { return myit(start_); }
	myit end() { return myit(end_); }
};

TEST(NumberSequce, range_for)
{
	for (auto n : Numbers(3, 5))
	{
		std::cout << n << ",";
	}
}

TEST(NumberSequce, vector_fill)
{
	Numbers nums(7, 10);
	std::vector<int> vec { std::begin(nums), std::end(nums) };
}
