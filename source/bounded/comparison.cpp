// Copyright David Stone 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

export module bounded.comparison;

import bounded.builtin_min_max_value;
import bounded.is_bounded_integer;

import numeric_traits;
import std_module;

namespace bounded {
namespace detail {

constexpr auto max_signed = numeric_traits::max_value<numeric_traits::max_signed_t>;

export template<builtin_integer LHS, builtin_integer RHS>
constexpr auto safe_compare(LHS const lhs, RHS const rhs) -> std::strong_ordering {
	if constexpr (signed_builtin<LHS> == signed_builtin<RHS>) {
		return lhs <=> rhs;
	} else if constexpr (numeric_traits::max_value<LHS> <= max_signed and numeric_traits::max_value<RHS> <= max_signed) {
		return static_cast<numeric_traits::max_signed_t>(lhs) <=> static_cast<numeric_traits::max_signed_t>(rhs);
	} else if constexpr (signed_builtin<LHS>) {
		return lhs < 0 ? std::strong_ordering::less : static_cast<RHS>(lhs) <=> rhs;
	} else {
		return rhs < 0 ? std::strong_ordering::greater : lhs <=> static_cast<LHS>(rhs);
	}
}

export template<builtin_integer LHS, builtin_integer RHS>
constexpr auto safe_equal(LHS const lhs, RHS const rhs) -> bool {
	if constexpr (signed_builtin<LHS> == signed_builtin<RHS>) {
		return lhs == rhs;
	} else if constexpr (numeric_traits::max_value<LHS> <= max_signed and numeric_traits::max_value<RHS> <= max_signed) {
		return static_cast<numeric_traits::max_signed_t>(lhs) == static_cast<numeric_traits::max_signed_t>(rhs);
	} else if constexpr (signed_builtin<LHS>) {
		static_assert(std::same_as<RHS, numeric_traits::max_unsigned_t>);
		return lhs >= 0 and static_cast<RHS>(lhs) == rhs;
	} else {
		static_assert(std::same_as<LHS, numeric_traits::max_unsigned_t>);
		return rhs >= 0 and lhs == static_cast<LHS>(rhs);
	}
}

} // namespace detail

export template<bounded_integer LHS, bounded_integer RHS>
constexpr auto operator<=>(LHS const lhs, RHS const rhs) -> std::strong_ordering {
	constexpr auto lhs_min = builtin_min_value<LHS>;
	constexpr auto lhs_max = builtin_max_value<LHS>;
	constexpr auto rhs_min = builtin_min_value<RHS>;
	constexpr auto rhs_max = builtin_max_value<RHS>;
	if constexpr (detail::safe_compare(lhs_min, rhs_max) > 0) {
		return std::strong_ordering::greater;
	} else if constexpr (detail::safe_compare(lhs_max, rhs_min) < 0) {
		return std::strong_ordering::less;
	} else if constexpr (detail::safe_equal(lhs_min, lhs_max) and detail::safe_equal(rhs_min, rhs_max) and detail::safe_equal(lhs_min, rhs_min)) {
		return std::strong_ordering::equal;
	} else {
		return detail::safe_compare(lhs.value(), rhs.value());
	}
}

export template<bounded_integer LHS, bounded_integer RHS>
constexpr auto operator==(LHS const lhs, RHS const rhs) -> bool {
	constexpr auto lhs_min = builtin_min_value<LHS>;
	constexpr auto lhs_max = builtin_max_value<LHS>;
	constexpr auto rhs_min = builtin_min_value<RHS>;
	constexpr auto rhs_max = builtin_max_value<RHS>;
	if constexpr (detail::safe_compare(lhs_min, rhs_max) > 0 or detail::safe_compare(lhs_max, rhs_min) < 0) {
		return false;
	} else if constexpr (detail::safe_equal(lhs_min, lhs_max) and detail::safe_equal(rhs_min, rhs_max) and detail::safe_equal(lhs_min, rhs_min)) {
		return true;
	} else {
		return detail::safe_equal(lhs.value(), rhs.value());
	}
}

export template<typename LHS, typename RHS = LHS>
concept equality_comparable = requires(LHS const & lhs, RHS const & rhs) { lhs == rhs; };

export template<typename LHS, typename RHS = LHS>
concept ordered = requires(LHS const & lhs, RHS const & rhs) { lhs <=> rhs; };

} // namespace bounded
