// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/algorithms/advance.hpp>
#include <containers/reference_wrapper.hpp>

#include <bounded/detail/forward.hpp>

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace containers {
namespace detail {

template<typename Range, typename ForwardIterator1, typename ForwardIterator2>
struct set_intersection_pair_iterator {
private:
	static constexpr auto max_difference = bounded::min(
		std::numeric_limits<typename std::iterator_traits<ForwardIterator1>::difference_type>::max(),
		std::numeric_limits<typename std::iterator_traits<ForwardIterator2>::difference_type>::max()
	);
public:
	using value_type = std::pair<
		decltype(	*std::declval<ForwardIterator1>()),
		decltype(*std::declval<ForwardIterator2>())
	>;
	using difference_type = bounded::integer<
		bounded::detail::normalize<static_cast<std::intmax_t>(-max_difference)>,
		bounded::detail::normalize<static_cast<std::intmax_t>(max_difference)>
	>;
	using pointer = value_type *;
	using reference = value_type;
	using iterator_category = std::forward_iterator_tag;
	
	constexpr set_intersection_pair_iterator(Range & range, ForwardIterator1 it1, ForwardIterator2 it2):
		m_range(range),
		m_it1(it1),
		m_it2(it2)
	{
	}

	constexpr auto & range() const {
		return m_range.get();
	}
	constexpr auto first() const {
		return m_it1;
	}
	constexpr auto second() const {
		return m_it2;
	}
	
private:
	reference_wrapper<Range> m_range;	
	ForwardIterator1 m_it1;
	ForwardIterator2 m_it2;
};

template<typename Range, typename ForwardIterator1, typename ForwardIterator2>
constexpr auto operator*(set_intersection_pair_iterator<Range, ForwardIterator1, ForwardIterator2> const it) {
	return typename set_intersection_pair_iterator<Range, ForwardIterator1, ForwardIterator2>::value_type(*it.first(), *it.second());
}

template<typename Range, typename ForwardIterator1, typename ForwardIterator2>
constexpr auto operator+(
	set_intersection_pair_iterator<Range, ForwardIterator1, ForwardIterator2> const lhs,
	bounded::constant_t<1>
) -> set_intersection_pair_iterator<Range, ForwardIterator1, ForwardIterator2> {
	return Range::find_first_matching(lhs.range(), containers::next(lhs.first()), containers::next(lhs.second()));
}

// These functions can compare only one pair of iterators because it is not
// legal to compare iterators from two different ranges. We know that if one
// set matches, the other must.
template<
	typename LHSForwardIterator1, typename LHSForwardIterator2, typename LHSRange,
	typename RHSForwardIterator1, typename RHSForwardIterator2, typename RHSRange,
	BOUNDED_REQUIRES(std::is_same_v<std::remove_const_t<LHSRange>, std::remove_const_t<RHSRange>>)
>
constexpr auto compare(
	set_intersection_pair_iterator<LHSForwardIterator1, LHSForwardIterator2, LHSRange> const lhs,
	set_intersection_pair_iterator<RHSForwardIterator1, RHSForwardIterator2, RHSRange> const rhs
) BOUNDED_NOEXCEPT_VALUE(
	compare(lhs.first(), rhs.first())
)
template<
	typename LHSForwardIterator1, typename LHSForwardIterator2, typename LHSRange,
	typename RHSForwardIterator1, typename RHSForwardIterator2, typename RHSRange,
	BOUNDED_REQUIRES(std::is_same_v<std::remove_const_t<LHSRange>, std::remove_const_t<RHSRange>>)
>
constexpr auto operator==(
	set_intersection_pair_iterator<LHSForwardIterator1, LHSForwardIterator2, LHSRange> const lhs,
	set_intersection_pair_iterator<RHSForwardIterator1, RHSForwardIterator2, RHSRange> const rhs
) BOUNDED_NOEXCEPT_VALUE(
	lhs.first() == rhs.first()
)

} // namespace detail

// set_intersection_pair is like set_intersection, but it returns both iterators
// TODO: find a better name for this
// TODO: This would benefit from a real compare function instead of less than
template<typename Range1, typename Range2, typename Compare>
struct set_intersection_pair {
	using iterator = detail::set_intersection_pair_iterator<
		set_intersection_pair<Range1, Range2, Compare>,
		decltype(begin(std::declval<Range1 &>())),
		decltype(begin(std::declval<Range2 &>()))
	>;
	using const_iterator = detail::set_intersection_pair_iterator<
		set_intersection_pair<Range1, Range2, Compare> const,
		decltype(begin(std::declval<Range1 const &>())),
		decltype(begin(std::declval<Range2 const &>()))
	>;
	using value_type = decltype(*std::declval<iterator>());
	using size_type = bounded::integer<
		0,
		bounded::detail::normalize<iterator::difference_type::max().value()>
	>;

	template<typename R1, typename R2>
	constexpr set_intersection_pair(R1 && range1, R2 && range2, Compare compare):
		m_range1(BOUNDED_FORWARD(range1)),
		m_range2(BOUNDED_FORWARD(range2)),
		m_compare(std::move(compare))
	{
	}
	
	template<typename R1, typename R2>
	constexpr set_intersection_pair(R1 && range1, R2 && range2):
		m_range1(BOUNDED_FORWARD(range1)),
		m_range2(BOUNDED_FORWARD(range2))
	{
	}
	
	friend constexpr auto begin(set_intersection_pair const & range) {
		return find_first_matching(range, begin(range.m_range1), begin(range.m_range2));
	}
	friend constexpr auto begin(set_intersection_pair & range) {
		return find_first_matching(range, begin(range.m_range1), begin(range.m_range2));
	}

	friend constexpr auto end(set_intersection_pair const & range) {
		return const_iterator(range, end(range.m_range1), end(range.m_range2));
	}
	friend constexpr auto end(set_intersection_pair & range) {
		return iterator(range, end(range.m_range1), end(range.m_range2));
	}

private:
	friend constexpr iterator detail::operator+<
		set_intersection_pair<Range1, Range2, Compare>,
		decltype(begin(std::declval<Range1 &>())),
		decltype(begin(std::declval<Range2 &>()))
	>(	iterator const lhs, bounded::constant_t<1>);
	friend constexpr const_iterator detail::operator+<
		set_intersection_pair<Range1, Range2, Compare> const,
		decltype(begin(std::declval<Range1 const &>())),
		decltype(begin(std::declval<Range2 const &>()))
	>(const_iterator const lhs, bounded::constant_t<1>);

	template<typename Self, typename Iterator1, typename Iterator2>
	static constexpr auto find_first_matching(Self & range, Iterator1 it1, Iterator2 it2) {
		auto const last1 = end(range.m_range1);
		auto const last2 = end(range.m_range2);
		auto const & comp = range.m_compare;
		
		while (it1 != last1 and it2 != last2) {
			if (comp(*it1, *it2)) {
				++it1;
			} else if (comp(*it2, *it1)) {
				++it2;
			} else {
				return detail::set_intersection_pair_iterator(range, it1, it2);
			}
		}
		return detail::set_intersection_pair_iterator(range, last1, last2);
	}

	Range1 m_range1;
	Range2 m_range2;
	Compare m_compare;
};

template<typename Range1, typename Range2, typename Compare>
set_intersection_pair(Range1 &&, Range2 &&, Compare) -> set_intersection_pair<Range1, Range2, Compare>;

template<typename Range1, typename Range2>
set_intersection_pair(Range1 &&, Range2 &&) -> set_intersection_pair<Range1, Range2, bounded::detail::less_t>;

}	// namespace containers
