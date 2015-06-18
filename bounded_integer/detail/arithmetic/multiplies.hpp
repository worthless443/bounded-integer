// Multiplication operator overload when each argument is a bounded::integer
// Copyright (C) 2015 David Stone
//
// This program is free software: you can redistribute it and / or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "base.hpp"

#include "../minmax.hpp"

namespace bounded {
namespace detail {

template<typename LHS, typename RHS>
constexpr auto multiplies_operator_range(LHS const & lhs, RHS const & rhs) noexcept {
	auto const p0 = lhs.min * rhs.min;
	auto const p1 = lhs.min * rhs.max;
	auto const p2 = lhs.max * rhs.min;
	auto const p3 = lhs.max * rhs.max;
	return min_max(
		min(p0, p1, p2, p3),
		max(p0, p1, p2, p3)
	);
}

}	// namespace detail

BOUNDED_INTEGER_OPERATOR_OVERLOADS(*, detail::multiplies_operator_range)

}	// namespace bounded
