// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "class.hpp"

#include <functional>

namespace std {

template<intmax_t minimum, intmax_t maximum, typename overflow_policy, bounded::storage_type storage, bool poisoned>
struct hash<bounded::integer<minimum, maximum, overflow_policy, storage, poisoned>> {
	using argument_type = bounded::integer<minimum, maximum, overflow_policy, storage, poisoned>;
	constexpr decltype(auto) operator()(argument_type const & argument) const {
		return hash<typename argument_type::underlying_type>{}(argument.value());
	}
};

}	// namespace std
