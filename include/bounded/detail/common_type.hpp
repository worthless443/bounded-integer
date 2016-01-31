// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <bounded/detail/forward_declaration.hpp>
#include <bounded/detail/policy/common_policy.hpp>

#include <type_traits>

namespace bounded {
namespace detail {

// This preferentially uses storage_type::fast, but does no conversions if they
// are the same. If new storage types are added, this will preferentially use
// storage_type::least over whatever that new type is.
constexpr storage_type common_storage_type(storage_type const lhs_storage, storage_type const rhs_storage) noexcept {
	return (lhs_storage < rhs_storage) ? lhs_storage : rhs_storage;
}

}	// namespace detail
}	// namespace bounded

namespace std {

// I do not have to specialize the single-argument version, as it just returns
// the type passed in, which will always work.

template<
	intmax_t lhs_min, intmax_t lhs_max, typename lhs_policy, bounded::storage_type lhs_storage, bool lhs_poisoned,
	intmax_t rhs_min, intmax_t rhs_max, typename rhs_policy, bounded::storage_type rhs_storage, bool rhs_poisoned
>
struct common_type<
	bounded::integer<lhs_min, lhs_max, lhs_policy, lhs_storage, lhs_poisoned>,
	bounded::integer<rhs_min, rhs_max, rhs_policy, rhs_storage, rhs_poisoned>
> {
private:
	static constexpr auto minimum = (lhs_min < rhs_min) ? lhs_min : rhs_min;
	static constexpr auto maximum = (lhs_max > rhs_max) ? lhs_max : rhs_max;
public:
	using type = bounded::integer<
		minimum,
		maximum,
		bounded::common_policy_t<lhs_policy, rhs_policy>,
		bounded::detail::common_storage_type(lhs_storage, rhs_storage),
		lhs_poisoned or rhs_poisoned
	>;
};


// We use common_type heavily in the creation of deduced arrays, so we need to
// add in some tricks to limit the maximum instantiation depth:

template<
	intmax_t minimum, intmax_t maximum, typename overflow_policy, bounded::storage_type storage, bool poisoned,
	typename T1, typename T2, typename T3, typename T4, typename T5,
	typename T6, typename T7, typename T8, typename T9, typename T10,
	typename T11, typename T12, typename T13, typename T14, typename T15,
	typename T16, typename T17, typename T18, typename T19, typename T20,
	typename T21, typename T22, typename T23, typename T24, typename T25,
	typename T26, typename T27, typename T28, typename T29, typename T30,
	typename T31, typename T32, typename T33, typename T34, typename T35,
	typename T36, typename T37, typename T38, typename T39, typename T40,
	typename T41, typename T42, typename T43, typename T44, typename T45,
	typename T46, typename T47, typename T48, typename T49, typename T50,
	typename... Ts
>
struct common_type<
	bounded::integer<minimum, maximum, overflow_policy, storage, poisoned>,
	T1, T2, T3, T4, T5, T6, T7, T8, T9, T10,
	T11, T12, T13, T14, T15, T16, T17, T18, T19, T20,
	T21, T22, T23, T24, T25, T26, T27, T28, T29, T30,
	T31, T32, T33, T34, T35, T36, T37, T38, T39, T40,
	T41, T42, T43, T44, T45, T46, T47, T48, T49, T50,
	Ts...
> {
private:
	using type0 = bounded::integer<minimum, maximum, overflow_policy, storage, poisoned>;
public:
	using type = common_type_t<
		common_type_t<
			type0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
			T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
			T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
			T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
			T40, T41, T42, T43, T44, T45, T46, T47, T48, T49
		>, T50, Ts...
	>;
};

}	// namespace std
