// Copyright David Stone 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#include <bounded/assert.hpp>

#include <operators/forward.hpp>

export module containers.unsafe_lazy_push_back;

import containers.begin_end;
import containers.can_set_size;
import containers.is_container;
import containers.member_lazy_push_backable;
import containers.range_value_t;
import containers.size;

import bounded;

using namespace bounded::literal;

namespace containers {

export template<typename Container>
concept unsafe_lazy_push_backable = is_container<Container> and (can_set_size<Container> or member_lazy_push_backable<Container>);

export template<unsafe_lazy_push_backable Container>
constexpr auto unsafe_lazy_push_back(
	Container & container,
	bounded::construct_function_for<range_value_t<Container>> auto && constructor
) -> auto & {
	if constexpr (can_set_size<Container>) {
		auto const initial_size = containers::size(container);
		BOUNDED_ASSERT(initial_size < container.capacity());
		auto & result = bounded::construct_at(*containers::end(container), OPERATORS_FORWARD(constructor));
		container.set_size(initial_size + 1_bi);
		return result;
	} else {
		return container.lazy_push_back(OPERATORS_FORWARD(constructor));
	}
}

} // namespace containers
