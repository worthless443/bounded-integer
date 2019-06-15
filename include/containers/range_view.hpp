// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/is_range.hpp>

#include <bounded/integer.hpp>

#include <iterator>
#include <utility>

namespace containers {

template<typename Iterator, typename Sentinel = Iterator>
struct range_view {
	using value_type = typename std::iterator_traits<Iterator>::value_type;

	using size_type = bounded::integer<
		0,
		bounded::detail::normalize<static_cast<std::uintmax_t>(std::numeric_limits<typename std::iterator_traits<Iterator>::difference_type>::max())>
	>;
	
	using const_iterator = Iterator;
	using iterator = Iterator;

	constexpr range_view(Iterator first, Sentinel last) noexcept(std::is_nothrow_move_constructible_v<Iterator> and std::is_nothrow_move_constructible_v<Sentinel>):
		m_begin(std::move(first)),
		m_end(std::move(last))
	{
	}
	constexpr explicit range_view(std::pair<Iterator, Sentinel> pair) BOUNDED_NOEXCEPT_INITIALIZATION(
		range_view(std::move(pair).first, std::move(pair).second)
	) {
	}
	template<typename Range> requires is_range<Range>
	constexpr explicit range_view(Range & range) BOUNDED_NOEXCEPT_INITIALIZATION(
		range_view(begin(range), end(range))
	) {
	}
	
	friend constexpr Iterator begin(range_view view) {
		return view.m_begin;
	}
	friend constexpr Sentinel end(range_view view) {
		return view.m_end;
	}
	
private:
	Iterator m_begin;
	Sentinel m_end;
};

template<typename Range>
range_view(Range &) -> range_view<decltype(begin(std::declval<Range &>()), end(std::declval<Range &>()))>;

template<typename Iterator, typename Sentinel>
constexpr auto operator==(range_view<Iterator, Sentinel> const lhs, range_view<Iterator, Sentinel> const rhs) BOUNDED_NOEXCEPT_VALUE(
	begin(lhs) == begin(rhs) and end(lhs) == end(rhs)
)

template<typename>
constexpr auto is_range_view = false;

template<typename Iterator, typename Sentinel>
constexpr auto is_range_view<range_view<Iterator, Sentinel>> = true;

} // namespace containers
