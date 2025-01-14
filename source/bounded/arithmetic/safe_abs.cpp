// Copyright David Stone 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

export module bounded.arithmetic.safe_abs;

import numeric_traits;

namespace bounded::detail {

export constexpr auto safe_abs = [](auto const value) {
	return (value < 0) ? -static_cast<numeric_traits::max_unsigned_t>(value) : static_cast<numeric_traits::max_unsigned_t>(value);
};

} // namespace bounded::detail
