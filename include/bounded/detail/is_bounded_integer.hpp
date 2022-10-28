// Copyright David Stone 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <numeric_traits/int128.hpp>

#include <concepts>
#include <type_traits>

namespace bounded {

template<auto minimum, auto maximum>
struct integer;

namespace detail {

template<typename T>
inline constexpr auto is_bounded_integer = false;

template<typename T>
inline constexpr auto is_bounded_integer<T const> = is_bounded_integer<T>;

template<typename T>
inline constexpr auto is_bounded_integer<T volatile> = is_bounded_integer<T>;

template<typename T>
inline constexpr auto is_bounded_integer<T const volatile> = is_bounded_integer<T>;

template<typename T>
inline constexpr auto is_bounded_integer<T &> = is_bounded_integer<T>;

template<typename T>
inline constexpr auto is_bounded_integer<T &&> = is_bounded_integer<T>;

template<auto minimum, auto maximum>
inline constexpr auto is_bounded_integer<integer<minimum, maximum>> = true;

} // namespace detail

template<typename T>
concept bounded_integer = detail::is_bounded_integer<T>;

namespace detail {

template<typename T>
concept signed_builtin = numeric_traits::signed_builtin<T>;

template<typename T>
concept unsigned_builtin = numeric_traits::unsigned_builtin<T> and !std::same_as<T, bool>;

template<typename T>
concept builtin_integer = signed_builtin<T> or unsigned_builtin<T>;

template<typename T>
inline constexpr auto is_integral_constant_of_integral = false;

template<typename T, T value>
inline constexpr auto is_integral_constant_of_integral<std::integral_constant<T, value>> = builtin_integer<T>;

} // namespace detail

template<typename T>
concept integral = detail::builtin_integer<T> or bounded_integer<T> or detail::is_integral_constant_of_integral<T>;

template<typename T>
concept isomorphic_to_integral = integral<T> or std::is_enum_v<T> or std::same_as<T, bool>;

} // namespace bounded
