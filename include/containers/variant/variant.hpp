// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/tuple.hpp>
#include <containers/type.hpp>
#include <containers/variant/special_member_functions.hpp>
#include <containers/variant/visit.hpp>

#include <cassert>
#include <type_traits>

namespace containers {

template<typename GetFunction, typename... Ts>
struct basic_variant :
	private detail::variant_copy_assignment<GetFunction, Ts...>
{
private:
	using base = detail::variant_copy_assignment<GetFunction, Ts...>;
public:
	static_assert(std::is_trivially_copyable<GetFunction>{});

	using base::base;
	
	using base::index;
	using base::operator[];
	using base::emplace;
};



namespace detail {

template<std::size_t size>
struct variant_selector {
	template<auto n>
	constexpr explicit variant_selector(bounded::constant_t<n> index_) noexcept:
		index(index_)
	{
	}
	template<typename Union>
	constexpr auto operator()(Union const &) const noexcept {
		return index;
	}
private:
	bounded::integer<0, size - 1> index;
};

template<>
struct variant_selector<0> {
};

}	// namespace detail

template<typename... Ts>
using variant = basic_variant<detail::variant_selector<sizeof...(Ts)>, Ts...>;


}	// namespace containers
