// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <bounded/detail/forward.hpp>
#include <bounded/integer.hpp>

#include <type_traits>

namespace containers {

template<typename Predicate>
constexpr auto negate(Predicate predicate) noexcept(std::is_nothrow_move_constructible_v<decltype(predicate)>) {
	return [predicate = std::move(predicate)](auto && ... args) BOUNDED_NOEXCEPT(
		!predicate(BOUNDED_FORWARD(args)...)
	);
}

}	// namespace containers
