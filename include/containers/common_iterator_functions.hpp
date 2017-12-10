// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/common_functions.hpp>
#include <containers/is_iterator.hpp>

#include <bounded/integer.hpp>

// The strange namespacing and using declarations here are to ensure these
// functions are picked up by ADL for types defined in namespaces ::container or
// ::container::detail

namespace containers {
namespace detail {

// The value of a difference is equal to the largest possible size for
// std::distance(begin(), end()), and it is equal to the negation of that value
// for std::distance(end(), begin())
template<typename Size>
using basic_difference_type = std::common_type_t<
	decltype(-std::declval<Size>()),
	Size
>;

namespace common {

template<typename Offset, typename Iterator, BOUNDED_REQUIRES(std::numeric_limits<Offset>::is_integer and is_iterator<Iterator>)>
constexpr auto operator+(Offset const offset, Iterator const it) BOUNDED_NOEXCEPT(
	it + offset
)

}	// namespace common

using containers::detail::common::operator+;

}	// namespace detail

using containers::detail::common::operator+;

}	// namespace containers
