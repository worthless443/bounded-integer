// Copyright David Stone 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

namespace containers {

template<typename T, std::size_t size>
using c_array = T[size];

// Using this as a parameter allows construction from `{}` to work. This is
// necessary because C arrays cannot be 0 size. The recommended pattern is
// ```
// template<std::same_as<empty_c_array_parameter> Source = empty_c_array_parameter>
// constexpr container(Source) {
// }
// ```
// This avoids ambiguity with the move constructor in `container({})` (by making
// it resolve to move constructing a default constructed container) but still
// allows implicit conversions from `{}` (calls the default constructor) and
// `{{}}` (calls this constructor). With constexpr function parameters, this can
// be replaced with a constexpr `std::initializer_list` parameter constrained to
// have a size of 0 and then `container({})` won't move construct a defaulted
// constructed container.

struct empty_c_array_parameter {
};

} // namespace containers
