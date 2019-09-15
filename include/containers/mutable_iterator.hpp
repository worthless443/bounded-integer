// Copyright David Stone 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/begin_end.hpp>
#include <containers/is_range.hpp>
#include <containers/is_iterator.hpp>

#include <bounded/integer.hpp>

namespace containers {
namespace detail {

template<range Range, iterator Iterator>
constexpr auto mutable_iterator(Range & range, Iterator const it) {
	return begin(range) + (it - begin(range));
}

}	// namespace detail
}	// namespace containers
