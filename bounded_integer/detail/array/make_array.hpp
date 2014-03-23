// Make an array with automatically deduced size and type
// Copyright (C) 2014 David Stone
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

#ifndef BOUNDED_INTEGER_DETAIL_MAKE_ARRAY_HPP_
#define BOUNDED_INTEGER_DETAIL_MAKE_ARRAY_HPP_

#include "array.hpp"
#include "../common_type.hpp"
#include <cstddef>
#include <utility>

namespace bounded_integer {
namespace detail {

template<typename T, std::size_t... dimensions>
class array_c;

template<typename T, std::size_t... dimensions>
using array_type = typename array_c<T, dimensions...>::type;

template<typename T, std::size_t dimension, std::size_t... dimensions>
class array_c<T, dimension, dimensions...> {
public:
	using type = array<array_type<T, dimensions...>, dimension>;
};

template<typename T>
class array_c<T> {
public:
	using type = T;
};

template<std::size_t... dimensions>
class dimension_product;

template<std::size_t dimension, std::size_t... dimensions>
class dimension_product<dimension, dimensions...> {
public:
	static constexpr std::size_t value = dimension * dimension_product<dimensions...>::value;
};

template<>
class dimension_product<> {
public:
	static constexpr std::size_t value = 1;
};

template<std::size_t number_of_values, std::size_t... dimensions>
class final_dimension {
private:
	static constexpr std::size_t product = dimension_product<dimensions...>::value;
public:
	static_assert(number_of_values % product == 0, "Incorrect number of parameters to deduce dimension.");
	static constexpr std::size_t value = number_of_values / product;
};

}	// namespace detail

// If element_type is not specified, the type is deduced as common_type.
//
// This has some code duplication (rather than all versions forwarding to the
// explicitly typed overload of make_explicit_array) because every time you
// forward a large number of arguments, gcc and clang both use up a lot of
// memory. For creating an array of around 4000 elements, these functions were
// unusable when they did not all construct detail::array_type directly.

// These assume that all of the dimensions have been passed in.
template<typename element_type, std::size_t... dimensions, typename... Args>
constexpr auto make_explicit_array(Args && ... args) noexcept {
	return detail::array_type<element_type, dimensions...>{ std::forward<Args>(args)... };
}
template<std::size_t... dimensions, typename... Args>
constexpr auto make_explicit_array(Args && ... args) noexcept {
	return detail::array_type<std::common_type_t<Args...>, dimensions...>{ std::forward<Args>(args)... };
}


// These assume you did not specify the inner-most dimension.
template<typename element_type, std::size_t... dimensions, typename... Args>
constexpr auto make_array(Args && ... args) noexcept {
	return detail::array_type<
		element_type,
		detail::final_dimension<sizeof...(Args), dimensions...>::value, dimensions...
	>{ std::forward<Args>(args)... };
}

template<std::size_t... dimensions, typename... Args>
constexpr auto make_array(Args && ... args) noexcept {
	return detail::array_type<
		std::common_type_t<Args...>,
		detail::final_dimension<sizeof...(Args), dimensions...>::value, dimensions...
	>{ std::forward<Args>(args)... };
}


}	// namespace bounded_integer
#endif	// BOUNDED_INTEGER_DETAIL_MAKE_ARRAY_HPP_
