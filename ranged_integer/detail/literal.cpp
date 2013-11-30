// Verify that the header can stand on its own, run tests
// Copyright (C) 2013 David Stone
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

#include "literal.hpp"
#include "comparison_operators.hpp"

namespace {

// I have to use the preprocessor here to create an integer literal
#define RANGED_INTEGER_CHECK_LITERAL(x) \
	static_assert(std::numeric_limits<decltype(x ## _ri)>::min() == std::numeric_limits<decltype(x ## _ri)>::max(), "Literal does not have a min possible value equal to a max possible value."); \
	static_assert(std::numeric_limits<decltype(x ## _ri)>::min() == x ## _ri, "Literal does not have a value equal to the range."); \
	\
	static_assert(x ## _ri == static_cast<decltype(x ## _ri)::underlying_type>(x), "Inaccurate value of " #x " (cast x)"); \
	static_assert(static_cast<decltype(x)>(x ## _ri) == x, "Inaccurate value of " #x " (cast value)");

RANGED_INTEGER_CHECK_LITERAL(0)
RANGED_INTEGER_CHECK_LITERAL(1)
RANGED_INTEGER_CHECK_LITERAL(10)
RANGED_INTEGER_CHECK_LITERAL(1000)
RANGED_INTEGER_CHECK_LITERAL(4294967295)
RANGED_INTEGER_CHECK_LITERAL(4294967296)
RANGED_INTEGER_CHECK_LITERAL(9223372036854775807)
RANGED_INTEGER_CHECK_LITERAL(-1)
RANGED_INTEGER_CHECK_LITERAL(-0)
#undef RANGED_INTEGER_CHECK_LITERAL

}	// namespace
