#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <gtest/gtest.h>

// https://www.artificialworlds.net/blog/2017/05/12/c-iterator-wrapperadaptor-example/
namespace {

class myit
{
private:

	std::string::const_iterator wrapped_;

public:

	// Previously provided by std::iterator (deprecated since c++17)
	using value_type = int;
	using difference_type = std::ptrdiff_t;
	using pointer = int*;
	using reference = int&;
	using iterator_category = std::input_iterator_tag;

	explicit myit(std::string::const_iterator wrapped) : wrapped_(wrapped) {}
	int operator*() const { return std::toupper(*wrapped_); }
	bool operator==(const myit& other) const { return wrapped_ == other.wrapped_; }
	bool operator!=(const myit& other) const { return !(*this == other); }

	myit operator++(int)
	{
		myit ret(wrapped_);
		++*this;
		return ret;
	}

	myit& operator++()
	{
		++wrapped_;
		return *this;
	}
};

class Upper
{
public:

	Upper(const std::string& str) : str_(str) {}
	myit begin() { return myit(str_.begin()); }
	myit end() { return myit(str_.end()); }

private:

	std::string str_;
};
}

TEST(Wrapper, Upper)
{
	for (auto ch : Upper("abcdef"))
	{
		// Prints "ABCDEF
		std::cout << ch;
	}
	std::cout << std::endl;

	Upper up(std::string("bAr"));
	std::string newfoo(std::begin(up), std::end(up));
	EXPECT_EQ(newfoo, "BAR");

	auto it = up.begin();
	EXPECT_EQ(*++(it++), 'A');
}
