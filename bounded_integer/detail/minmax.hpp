// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "basic_numeric_limits.hpp"
#include "common_type.hpp"
#include "common_type_and_value_category.hpp"
#include "comparison.hpp"
#include "comparison_function_object.hpp"
#include "is_bounded_integer.hpp"
#include "noexcept.hpp"
#include "overlapping_range.hpp"
#include "requires.hpp"
#include "policy/common_policy.hpp"

#include <utility>

namespace bounded {
namespace detail {
namespace minmax {

template<typename Target, typename Source, BOUNDED_REQUIRES(is_bounded_integer<Target> and not std::is_reference<Target>::value)>
constexpr Target construct(Source && source) noexcept {
	return Target(std::forward<Source>(source), non_check);
}
template<typename Target, typename Source, BOUNDED_REQUIRES(!is_bounded_integer<Target> or std::is_reference<Target>::value)>
constexpr Target construct(Source && source) noexcept(noexcept(static_cast<Target>(source))) {
	return static_cast<Target>(source);
}

}	// namespace minmax
}	// namespace detail


// Specialize this class to give the correct type for your own extreme function
// if the default does not work
template<typename Compare, typename T1, typename T2>
struct extreme_type {
	using type = std::common_type_t<T1, T2>;
};

template<typename Compare, typename T1, typename T2>
using extreme_t = typename extreme_type<Compare, std::decay_t<T1>, std::decay_t<T2>>::type;

// TODO: This should only be selected for less and greater
template<
	typename Compare,
	intmax_t lhs_min, intmax_t lhs_max, typename lhs_policy, storage_type lhs_storage, bool lhs_poisoned,
	intmax_t rhs_min, intmax_t rhs_max, typename rhs_policy, storage_type rhs_storage, bool rhs_poisoned
>
struct extreme_type<
	Compare,
	integer<lhs_min, lhs_max, lhs_policy, lhs_storage, lhs_poisoned>,
	integer<rhs_min, rhs_max, rhs_policy, rhs_storage, rhs_poisoned>
> {
private:
	static constexpr auto minimum = Compare{}(lhs_min, rhs_min) ? lhs_min : rhs_min;
	static constexpr auto maximum = Compare{}(lhs_max, rhs_max) ? lhs_max : rhs_max;
	using policy = common_policy_t<lhs_policy, rhs_policy>;
	static constexpr auto storage = detail::common_storage_type(lhs_storage, rhs_storage);
	static constexpr auto poisoned = lhs_poisoned or rhs_poisoned;
public:
	using type = integer<minimum, maximum, policy, storage, poisoned>;
};



// Unlike the standard versions of min and max, which accept a fixed number of
// arguments, these functions accept a variadic pack. Therefore, there is no way
// to distinguish whether the user wanted to compare all of the object with
// `operator<` or a comparison function. The separately named function `extreme`
// serves that purpose.
//
// Because the variadic pack must be at the end of the parameter list, `extreme`
// accepts the comparison function as the first argument

template<typename Compare, typename T>
constexpr decltype(auto) extreme(Compare, T && t) noexcept {
	return t;
}



// If the type of the result has no overlap with the type one of the arguments,
// then that argument cannot be the true value. Therefore, we can skip any
// actual comparison and just return the other value. The general code would not
// compile without this, because you cannot cast to a bounded::integer type that
// has no overlap.
template<typename Compare, typename T1, typename T2, BOUNDED_REQUIRES(
	basic_numeric_limits<T1>::is_specialized and basic_numeric_limits<T2>::is_specialized and
	not detail::types_overlap<extreme_t<Compare, T1, T2>, T2>::value
)>
constexpr decltype(auto) extreme(Compare, T1 && t1, T2 &&) noexcept {
	return detail::minmax::construct<T1>(std::forward<T1>(t1));
}
template<typename Compare, typename T1, typename T2, BOUNDED_REQUIRES(
	basic_numeric_limits<T1>::is_specialized and basic_numeric_limits<T2>::is_specialized and
	not detail::types_overlap<extreme_t<Compare, T1, T2>, T1>::value
)>
constexpr decltype(auto) extreme(Compare, T1 &&, T2 && t2) noexcept {
	return detail::minmax::construct<T2>(std::forward<T2>(t2));
}

// TODO: Fix noexcept specification
template<typename Compare, typename T1, typename T2, BOUNDED_REQUIRES(
	(not basic_numeric_limits<T1>::is_specialized or not basic_numeric_limits<T2>::is_specialized) or
	(detail::types_overlap<extreme_t<Compare, T1, T2>, T1>::value and
	detail::types_overlap<extreme_t<Compare, T1, T2>, T2>::value)
)>
constexpr decltype(auto) extreme(Compare compare, T1 && t1, T2 && t2) noexcept(noexcept(compare(t1, t2))) {
	using result_type = detail::add_common_cv_reference_t<extreme_t<Compare, T1, T2>, T1, T2>;
	return compare(t2, t1) ?
		detail::minmax::construct<result_type>(std::forward<T2>(t2)) :
		detail::minmax::construct<result_type>(std::forward<T1>(t1))
	;
}


namespace detail {
namespace minmax {

// These are needed because you cannot have a recursive noexcept specification
// https://stackoverflow.com/questions/23772928/recursive-noexcept-specification

template<typename Compare, typename... Ts>
struct noexcept_comparable;

template<typename Compare, typename T>
struct noexcept_comparable<Compare, T> : std::integral_constant<bool, true> {
};

template<typename Compare, typename T1, typename T2>
struct noexcept_comparable<Compare, T1, T2> : std::integral_constant<bool, noexcept(
	std::declval<Compare>()(std::declval<T1>(), std::declval<T2>())
)> {
};

template<typename Compare, typename T1, typename T2, typename... Ts>
struct noexcept_comparable<Compare, T1, T2, Ts...> : std::integral_constant<bool,
	noexcept_comparable<Compare, T1, T2>::value and
	noexcept_comparable<Compare, extreme_t<Compare, T1, T2>, Ts...>::value
> {
};

}	// namespace minmax
}	// namespace detail


// TODO: noexcept specification needs to take possible copies / moves into
// account if this supports non bounded::integer types.
template<typename Compare, typename T1, typename T2, typename... Ts>
constexpr decltype(auto) extreme(Compare compare, T1 && t1, T2 && t2, Ts && ... ts) noexcept(detail::minmax::noexcept_comparable<Compare, T1, T2, Ts...>::value) {
	return extreme(
		compare,
		extreme(compare, std::forward<T1>(t1), std::forward<T2>(t2)),
		std::forward<Ts>(ts)...
	);
}

template<typename... Ts>
constexpr auto min(Ts && ... ts) BOUNDED_NOEXCEPT_DECLTYPE(
	extreme(less(), std::forward<Ts>(ts)...)
)
template<typename... Ts>
constexpr auto max(Ts && ... ts) BOUNDED_NOEXCEPT_DECLTYPE(
	extreme(greater(), std::forward<Ts>(ts)...)
)

}	// namespace bounded
