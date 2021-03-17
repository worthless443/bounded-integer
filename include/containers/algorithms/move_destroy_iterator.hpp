// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/algorithms/transform_iterator.hpp>

#include <containers/move_destroy.hpp>

#include <type_traits>

namespace containers {

constexpr auto move_destroy_iterator(iterator auto it_) {
	return ::containers::transform_iterator_dereference(it_, [](decltype(it_) const it) noexcept {
		if constexpr (std::is_reference_v<decltype(*it)>) {
			return move_destroy(*it);
		} else {
			return *it;
		}
	});
}

}	// namespace containers
