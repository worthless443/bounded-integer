// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <bounded/detail/arithmetic/base.hpp>
#include <bounded/detail/max_builtin.hpp>

namespace bounded {
namespace detail {

constexpr auto left_shift = [](auto const lhs, auto const rhs) {
	return lhs << rhs;
};

} // namespace detail

template<
	auto lhs_min, auto lhs_max, typename lhs_policy,
	auto rhs_min, auto rhs_max, typename rhs_policy
>
constexpr auto operator<<(
	integer<lhs_min, lhs_max, lhs_policy> const lhs_,
	integer<rhs_min, rhs_max, rhs_policy> const rhs_
) {
	return detail::operator_overload(lhs_, rhs_, detail::left_shift, [](auto const lhs, auto const rhs) {
		// TODO: Broaden range
		return detail::min_max{
			static_cast<detail::max_signed_t>(lhs.min) << rhs.min.value(),
			static_cast<detail::max_signed_t>(lhs.max) << rhs.max.value()
		};
	});
}

}	// namespace bounded

