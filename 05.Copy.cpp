// CppCon 2016: "Building and Extending the Iterator Hierarchy in a Modern, Multicore World"
// https://en.cppreference.com/w/cpp/header/concepts

#include <iostream>
#include <cassert>
#include <array>
#include <string>
#include <vector>
#include <type_traits>

/*
 template <typename T>
 output copy(input sequence, output sequence)
 {
 	while(input not empty && output not empty)
		write output = read input;
		increment output;
		increment input;
	return output;
 }
 */

template <typename T>
struct node
{
	T value;
	node* next;
};

inline auto const& source(auto const* p) { return *p; }
inline auto const& source(node<auto> const* p) { return p->value; }
inline auto& sink(auto* p) { return *p; }
inline auto& sink(node<auto>* p) { return p->value; }
inline auto successor(auto* p) { return p + 1; }
inline auto successor(node<auto>* p) { return p->next; }
inline auto predecessor(auto* p) { return p - 1; }
inline auto predecessor(node<auto>* p) = delete;

					
template <typename InputIterator, typename OutputIterator>
OutputIterator copy(InputIterator in, InputIterator in_end, OutputIterator out, OutputIterator out_end)
{
	while (in != in_end && out != out_end)
	{
		sink(out) = source(in);
		out = successor(out);
		in = successor(in);
	}
	return out;
}


using namespace std;

// NOTE: std::Regular
/*
   template <class T>
   concept Regular = std::Semiregular<T> && std::EqualityComparable<T>;
 */
template <typename T>
concept bool Regular =
	std::is_default_constructible_v<T>
 && std::is_copy_constructible_v<T>
 && std::is_destructible_v<T>
 && std::is_copy_assignable_v<T>
 && requires(T x, T y) {
	 { x == y } -> bool;
	 // == induces an equivalence relation on T
	 // Iff x == y, x and y represent the same value
 };

// https://github.com/ericniebler/stl2/issues/423
/* value_type */
template <class> struct value_type { };

template <class T>
  requires is_array<T>::value
struct value_type<T> : value_type<decay_t<T>> { };

template <class T>
struct value_type<T const> : value_type<decay_t<T>> { };

template <class T>
   requires is_object<T>::value
struct value_type<T*> {
   using type = remove_cv_t<T>;
};

template <class T>
concept bool __MemberValueType = // exposition-only
  requires { typename T::value_type; };

template <__MemberValueType T>
struct value_type<T> { };

template <__MemberValueType T>
  requires is_object<typename T::value_type>::value
struct value_type<T> {
  using type = typename T::value_type;
};

template <class T> 
struct value_type<node<T>*>
{
	using type = decltype(node<T>::value);
};

template <class T> using value_type_t
  = typename value_type<T>::type;
/* value_type */

template <typename T>
concept bool Readable =
	requires (T x) {
		typename value_type_t<T>;
		{ source(x) } -> value_type_t<T> const&;
	};

template <typename T>
concept bool Writable =
	requires (T x) {
		typename value_type_t<T>;
		{ sink(x) } -> value_type_t<T> &;
	};

template <typename I>
concept bool Iterator = 
	Regular<I> &&
	requires (I i) {
		{ successor(i) } -> I;
	};

// Multi-pass Guarantee
// A ForwardIterator is just an Iterator that doesn’t mutate other iterators in successor()
template <typename I>
concept bool ForwardIterator =
	Regular<I> &&
	requires (I i) {
		{ successor(i) } -> I; // O(1)
	};

template <typename I>
concept bool BidirectionalIterator =
Regular<I> &&
requires (I i) {
	{ successor(i) } -> I; // O(1)
	{ predecessor(i) } -> I; // O(1)
	// i == predecessor(successor(i));
};

// TODO: Can I use StrictTotallyOrderedWith instead?
template <class R>
concept bool WeaklyOrdered = true;

template <typename I>
concept bool RandomAccessIterator =
	   Regular<I>
	&& WeaklyOrdered<I>
	&& requires (I i, I j, size_t n) {
		{ i + n } -> I; // O(1)
		// i + 0 == i if n == 0
		// i + n == successor(i) + n - 1 if n > 0
		// i + n == predecessor(i) + n + 1 if n < 0
		{ i - n } -> I; // O(1)
		// i - 0 == i if n == 0
		// i - n == predecessor(i) - (n - 1) if n > 0
		// i - n == successor(i) - (n + 1) if n < 0
		{ i - j } -> size_t; // O(1)
		// i + (i - j) = i
};


template <typename T>
concept bool ContiguousIterator =
RandomAccessIterator<T>
&& requires (T i) {
	typename value_type<T>;
	{ pointer_from(i) } -> value_type<T> const*;
	// pointer_from homomorphism preserves range
	// structure
};

template <Regular T>
struct segmented_array {
	vector< vector<T> > data;
	// where each inner vector except the last has size segsize
};

/* TODO:
 
template <typename T>
concept bool SegmentedIterator =
	Iterator<T>
	&& requires (T i) {
		typename local_iterator<T>;
		typename segment_iterator<T>;
		requires Iterator<local_iterator>;
		requires Iterator<segment_iterator>;
		requires Range<segment_iterator>; // begin(), end()
		{ local(i) } -> local_iterator<T>;
		{ segment(i) } -> segment_iterator<T>;
	};

template <SegmentedIterator In, Iterator Out>
	requires Readable<In> && Writable<Out>
Out copy(In in, In in_end, Out out, Out out_end)
{
	auto seg = segment(in);
	auto seg_end = segment(in_end);
	if (seg == seg_end) copy(local(in), local(in_end), out, out_end);
	else {
		out = copy(local(in), end(seg), out, out_end);
		seg = successor(seg);
		while (seg != seg_end) {
			out = copy(begin(seg), end(seg), out, out_end);
			seg = successor(seg);
		}
		return copy(begin(seg), local(in_end), out, out_end);
	}
}

template <RandomAccessIterator In, SegmentedIterator Out>
	requires Readable<In> && Writable<Out>
	      && RandomAccessIterator<Out>
Out copy(In in, In in_end, Out out, Out out_end)
{
	auto& task_pool = get_global_task_pool();
	auto seg = segment(out);
	auto seg_end = segment(out_end);
	if (seg == seg_end) {
		return copy(in, in_end, local(out), local(out_end));
	} else {
		task_pool.add_task(copy, in, in_end, local(out), end(seg_end));
		seg = successor(seg);
		in = in + min(in_end - in, end(seg_end) - local(out));
		while (seg != seg_end) {
			task_pool.add_task(copy, in, in_end, begin(seg), end(seg));
			seg = successor(seg);
			in = in + min(in_end - in, end(seg) - begin(seg));
		}
		task_pool.add_task(copy, in, in_end, begin(seg), local(out_end));
		return out + min(in_end - in, local(out_end) - begin(out));
	}
}

template <typename T>
concept bool CacheAwareIterator = SegmentedIterator<T> && ContiguousIterator<T>;

*/

namespace Concept {

template <Iterator In, Iterator Out>
	requires Readable<In> && Writable<Out>
Out copy(In in, In in_end, Out out, Out out_end)
{
	while (in != in_end && out != out_end)
	{
		sink(out) = source(in);
		out = successor(out);
		in = successor(in);
	}
	return out;
}

template <ContiguousIterator In, ContiguousIterator Out>
	requires Readable<In> && Writable<Out>
		  && is_same_v< value_type<In>, value_type<Out> >
	      && is_trivially_copyable_v< value_type<In> >
Out copy(In in, In in_end, Out out, Out out_end)
{
	auto count = min( in_end - in, out_end - out );
	memmove(pointer_from(out), pointer_from(in), count);
	return out_end;
}



template <Iterator It>
	requires Writable<It>
void fill(It it, It it_end, value_type<It> const& x)
{
	while (it != it_end)
	{
		sink(it) = x;
		it = successor(it);
	}
}

template <ForwardIterator It>
	requires Readable<It>
It max_element(It it, It it_end)
{
	auto max_it = it;
	while (it != it_end) {
		if (source(it) > source(max_it)) max_it = it;
		it = successor(it);
	}
	return max_it;
}

template <BidirectionalIterator I>
requires Readable<I> && Writable<I>
void reverse(I it_begin, I it_end) {
	while (it_begin != it_end) {
		it_end = predecessor(it_end);
		if (it_begin == it_end) break;
		auto temp = source(it_end);
		sink(it_end) = source(it_begin);
		sink(it_begin) = temp;
		it_begin = successor(it_begin);
	}
}

template <RandomAccessIterator It>
	requires Readable<It> && WeaklyOrdered<value_type_t<It>>
It upper_bound(It it, It it_end, value_type_t<It> x)
{
	// Base case.
	if (it == it_end) return it_end;
	// mid_dist is always less than or equal to end - begin,
	// because of integer division
	auto mid_dist = (it_end - it) / 2;
	auto mid = it + mid_dist;
	// Reduce problem size.
	if (source(mid) <= x) return upper_bound(mid + 1, it_end, x);
	else return upper_bound( it, mid + 1, x);
}

}

int main()
{
	// Test copy array
	{
		auto source = std::array<int, 5>{ 1, 2, 3, 4, 5 };
		auto destination = std::array<int, 7>{ 0, 0, 0, 0, 0, 0, 0 };

		copy(&source[0], &source[0]+source.size(), &destination[0], &destination[0]+destination.size());

		assert(destination[3] == 4);

		Concept::copy(&source[0], &source[0]+source.size(), &destination[0], &destination[0]+destination.size());
		assert(destination[3] == 4);
		assert(destination[4] == 5);
	}
	// Test copy node
	{
		auto destination = std::array<int, 7>{ 0, 0, 0, 0, 0, 0, 0 };
		node<int> last { 0 };
		std::array<node<int>, 3> l;
		l[0].value = 9;
		l[0].next = &l[1];
		l[1].value = 8;
		l[1].next = &l[2];
		l[2].value = 7;
		l[2].next = &last;
		Concept::copy(&l.front(), &last, &destination[0], &destination[0]+destination.size());
		assert(destination[0] == 9);
		assert(destination[1] == 8);
		assert(destination[2] == 7);
	}
	// Test max_element array
	{
		auto source = std::array<int, 5>{ 1, 2, 3, 4, 5 };
		auto it = Concept::max_element(source.begin(), source.end());
		assert(*it == 5);
	}
	// Test reverse array 
	{
		auto source = std::array<int, 5>{ 1, 2, 3, 4, 5 };
		Concept::reverse(source.begin(), source.end());
		assert(source[0] == 5);
	}
	return 0;
}
