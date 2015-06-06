// result_type of arithmetic operations
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

#include <cstdint>

namespace bounded {
namespace detail {

struct min_max {
	constexpr min_max(intmax_t min_, intmax_t max_) noexcept:
		min(min_),
		max(max_) {
	}
	intmax_t min;
	intmax_t max;
};

}	// namespace detail
}	// namespace bounded

