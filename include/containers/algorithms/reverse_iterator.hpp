// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/algorithms/advance.hpp>
#include <containers/begin_end.hpp>
#include <containers/is_iterator.hpp>
#include <containers/iterator_adapter.hpp>
#include <containers/adapt.hpp>
#include <containers/is_range.hpp>

#include <iterator>

namespace containers {
namespace detail {

struct reverse_traits {
	static constexpr auto get_begin(auto && range) {
		return end(BOUNDED_FORWARD(range));
	}

	static constexpr auto get_end(auto && range) {
		return begin(BOUNDED_FORWARD(range));
	}

	static constexpr decltype(auto) dereference(bidirectional_iterator auto it) {
		return *::containers::prev(it);
	}

	static constexpr auto add(random_access_iterator auto it, auto const offset) OPERATORS_RETURNS(
		it - offset
	)

	static constexpr auto subtract(random_access_iterator auto const lhs, random_access_iterator auto const rhs) {
		return rhs - lhs;
	}

	static constexpr auto compare(random_access_iterator auto const lhs, random_access_iterator auto const rhs) {
		return rhs <=> lhs;
	}
	static constexpr auto equal(iterator auto const lhs, iterator auto const rhs) {
		return rhs == lhs;
	}
};

}	// namespace detail

constexpr auto reverse_iterator(bidirectional_iterator auto it) {
	return adapt_iterator(it, detail::reverse_traits{});
}

constexpr auto reversed(range auto && source) {
	return adapt(BOUNDED_FORWARD(source), detail::reverse_traits{});
}

}	// namespace containers
