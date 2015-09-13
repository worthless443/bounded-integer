// A type suitable to index into a container
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

#include <bounded_integer/bounded_integer.hpp>

#include <limits>
#include <type_traits>

namespace containers {

template<typename Container>
using index_type = bounded::checked_integer<0, static_cast<std::intmax_t>(std::numeric_limits<typename std::remove_reference_t<Container>::size_type>::max()) - 1, std::out_of_range>;

}	// namespace containers
