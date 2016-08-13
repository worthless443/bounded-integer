// Copyright David Stone 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/is_container.hpp>

#include <cassert>
#include <type_traits>

// We currently cannot have a free-function `operator[]`. However, all sequence
// containers have the same implementation of it. This hides the boilerplate.

namespace containers {
namespace detail {

// This cannot use BOUNDED_NOEXCEPT_DECLTYPE because of the assert statement. On
// clang, it contains the predefined identifier __ASSERT_FUNCTION, which
// triggers an on-by-default warning: Wpredefined-identifier-outside-function.
//
// The alternative would be to write a new assert function that just gets
// __FILE__ and __LINE__, which may end up being a good idea if there are
// several functions that follow this pattern.

template<typename Container, BOUNDED_REQUIRES(is_container<Container>)>
constexpr auto operator_bracket(Container && container, index_type<std::decay_t<Container>> const index) noexcept(
	noexcept(index < size(container)) and
	noexcept(*(std::forward<Container>(container).begin() + index)) and
	std::is_nothrow_move_constructible<decltype(*(std::forward<Container>(container).begin() + index))>::value
) -> decltype(*(std::forward<Container>(container).begin() + index)) {
	assert(index < size(container));
	return *(std::forward<Container>(container).begin() + index);
}


}	// namespace detail
}	// namespace containers

#define CONTAINERS_OPERATOR_BRACKET_DEFINITIONS \
	template<typename Index> \
	constexpr auto operator[](Index const index) const & BOUNDED_NOEXCEPT_DECLTYPE( \
		::containers::detail::operator_bracket(*this, index) \
	) \
	template<typename Index> \
	constexpr auto operator[](Index const index) & BOUNDED_NOEXCEPT_DECLTYPE( \
		::containers::detail::operator_bracket(*this, index) \
	) \
	template<typename Index> \
	constexpr auto operator[](Index const index) && BOUNDED_NOEXCEPT_DECLTYPE( \
		::containers::detail::operator_bracket(std::move(*this), index) \
	)

