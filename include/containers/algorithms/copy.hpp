// Algorithms to copy that work with sentinels
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
#include <bounded_integer/integer_range.hpp>

namespace containers {


template<typename InputIterator, typename Sentinel, typename OutputIterator>
constexpr auto copy(InputIterator first, Sentinel const last, OutputIterator out) {
	struct result {
		InputIterator input;
		OutputIterator output;
	};
	for (; first != last; ++first) {
		*out = *first;
		++out;
	}
	return result { first, out };
}

// Ideally this would be implemented with a counted iterator
template<typename InputIterator, typename Size, typename OutputIterator>
constexpr auto copy_n(InputIterator first, Size const count, OutputIterator out) {
	static_assert(count >= bounded::constant<0>, "Negative numbers not supported.");
	struct result {
		InputIterator input;
		OutputIterator output;
	};
	for (auto const n : bounded::integer_range(count)) {
		static_cast<void>(n);
		*out = *first;
		++out;
		++first;
	}
	return result { first, out };
}

}	// namespace containers
