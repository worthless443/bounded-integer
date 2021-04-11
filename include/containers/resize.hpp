// Copyright David Stone 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/append.hpp>
#include <containers/pop_back.hpp>
#include <containers/range_value_t.hpp>
#include <containers/repeat_n.hpp>
#include <containers/resizable_container.hpp>
#include <containers/size.hpp>
#include <containers/take.hpp>

#include <operators/forward.hpp>
#include <bounded/detail/cast.hpp>
#include <bounded/integer.hpp>

#include <utility>

namespace containers {
namespace detail {

constexpr auto resize_impl(auto & container_to_resize, auto const initializer_range) {
	while (containers::size(container_to_resize) > containers::size(initializer_range)) {
		pop_back(container_to_resize);
	}
	auto const remaining = bounded::increase_min<0>(containers::size(initializer_range) - containers::size(container_to_resize), bounded::non_check);
	::containers::append(container_to_resize, containers::take(initializer_range, remaining));
}

} // namespace detail

template<resizable_container Container>
constexpr auto resize(Container & container_to_resize, auto const count) {
	::containers::detail::resize_impl(container_to_resize, repeat_default_n<range_value_t<Container>>(count));
}
template<resizable_container Container>
constexpr auto resize(Container & container_to_resize, auto const count, range_value_t<Container> const & value) {
	::containers::detail::resize_impl(container_to_resize, repeat_n(count, value));
}

} // namespace containers
