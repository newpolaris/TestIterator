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

	char* ch_;
	class chholder
	{

		char* ch_;

	public:

		chholder(char* ch) : ch_(ch) {}
		char operator*() { return std::toupper(*ch_); }
	};

public:

	// Previously provided by std::iterator (deprecated since c++17)
	using value_type = char;
	using difference_type = std::ptrdiff_t;
	using pointer = char*;
	using reference = char&;
	using iterator_category = std::input_iterator_tag;

	explicit myit(char* ch) : ch_(ch) {}
	char operator*() const { return std::toupper(*ch_); }
	bool operator==(const myit& other) const { return ch_ == other.ch_; }
	bool operator!=(const myit& other) const { return !(*this == other); }

	chholder operator++(int)
	{
		chholder ret(ch_);
		++*this;
		return ret;
	}

	myit& operator++()
	{
		++ch_;
		return *this;
	}
};

class Upper
{
public:

	Upper(const std::string& str) : str_(str) {}
	myit begin() { return myit(&str_[0]); }
	myit end() { return myit(&str_[str_.size()]); }

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

	EXPECT_EQ(*(++(++up.begin())), 'R');
}
