#include <iostream>
#include <iterator>
#include <string>
#include <gtest/gtest.h>

class myit
{

	struct maybestring
	{
		std::string value_;
		bool at_end_;

		explicit maybestring(const std::string& value) 
			: value_(value)
			, at_end_(false)
		{}

		explicit maybestring() 
			: value_("ERROR")
			, at_end_(true)
		{}
	};

	using iterator_type = std::string::const_iterator;
	iterator_type wrapped_;
	iterator_type wrapped_end_;
	maybestring next_;

	maybestring next_item()
	{
		if (wrapped_ == wrapped_end_)
			return maybestring();
		return read_item();
	}

	maybestring read_item()
	{
		std::string ret = "";
		for (; wrapped_ != wrapped_end_; ++wrapped_)
		{
			char c = *wrapped_;
			if (c == ':')
				break;
			ret += c;
		}
		for (; wrapped_ != wrapped_end_; ++wrapped_)
			if (*wrapped_ != ':')
				break;
		if (ret.size() == 0)
			return maybestring();
		return maybestring(ret);
	}

public:

	using value_type = std::string;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = std::input_iterator_tag;

	explicit myit(iterator_type wrapped, iterator_type wrapped_end) 
		: wrapped_(wrapped)
		, wrapped_end_(wrapped_end)
	{
		next_ = next_item();
	}

	value_type operator*() { return next_.value_; }
	bool operator==(const myit& other) const 
	{
		// We only care about whether we are at the end
		return next_.at_end_ == other.next_.at_end_;
	}
	bool operator!=(const myit& other) const { return !(*this == other); }

	myit operator++(int)
	{
		myit ret(wrapped_, wrapped_end_);
		next_ = next_item();
		return ret;
	}

	myit& operator++()
	{
		next_ = next_item();
		return *this;
	}

};

class ColonSep
{

public:

	ColonSep(const std::string& str) : str_(str) {}
	myit begin() { return myit(str_.begin(), str_.end()); }
	myit end() { return myit(str_.end(), str_.end()); }

private:

	const std::string str_;
};

TEST(ColonSep, ColonSep)
{
	ColonSep items("aa:foo::x");
	std::vector<std::string> strs(std::begin(items), std::end(items));

	EXPECT_EQ(strs.front(), "aa");
	EXPECT_EQ(strs.back(), "x");
}
