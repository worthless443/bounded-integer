// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/lazy_push_back.hpp>

#include <bounded/integer.hpp>
#include <bounded/value_to_function.hpp>

#include <utility>

namespace containers {
namespace detail {

template<typename Container>
concept member_push_backable = requires(Container & container, typename Container::value_type value) { container.push_back(std::move(value)); };

namespace common {

template<typename Container>
constexpr decltype(auto) push_back(Container & container, typename Container::value_type const & value) {
	if constexpr (member_push_backable<Container>) {
		container.push_back(value);
		return back(container);
	} else {
		return ::containers::lazy_push_back(
			container,
			::bounded::value_to_function(value)
		);
	}
}
template<typename Container>
constexpr decltype(auto) push_back(Container & container, typename Container::value_type && value) {
	if constexpr (member_push_backable<Container>) {
		container.push_back(std::move(value));
		return back(container);
	} else {
		return ::containers::lazy_push_back(
			container,
			::bounded::value_to_function(std::move(value))
		);
	}
}

}	// namespace common

using ::containers::detail::common::push_back;

}	// namespace detail

using ::containers::detail::common::push_back;

}	// namespace containers
