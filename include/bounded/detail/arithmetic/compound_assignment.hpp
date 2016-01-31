// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <bounded/detail/arithmetic/operators.hpp>
#include <bounded/detail/basic_numeric_limits.hpp>
#include <bounded/detail/forward_declaration.hpp>
#include <bounded/detail/is_bounded_integer.hpp>
#include <bounded/detail/make.hpp>
#include <bounded/detail/noexcept.hpp>
#include <bounded/detail/requires.hpp>

#include <type_traits>

namespace bounded {

// It seems as though it would be possible to write a common implementation of
// operator+= that can be used for any type that defines operator+ and
// operator=. The problem is with type compatibility. With a bounded::integer,
// the type returned by operator+ is not the same as the type of the left-hand
// side, which is what is assumed with the typical approach of implementing
// operator+ in terms of operator+=. The reverse is also problematic, however.
// It is also possible for two types to be valid arguments to operator+= without
// being valid arguments to operator+. This happens with bounded::integer with
// two different overflow policies and no defined relation -- for instance
// clamp_policy and modulo_policy. There is no 'best' type for operator+ to
// return when confronted with conflicting policies. However, for operator+=, we
// already know the result type is the type of the left-hand side, so we do not
// care about the policy of the right-hand side.
//
// An alternative is to change the definition of operator+ to always return
// null_policy, which would allow for a generic operator+= definition

#define BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(symbol) \
template<typename LHS, typename RHS, BOUNDED_REQUIRES(is_bounded_integer<LHS> and basic_numeric_limits<RHS>::is_integer)> \
constexpr auto operator symbol##=(LHS & lhs, RHS && rhs) BOUNDED_NOEXCEPT_DECLTYPE( \
	lhs = static_cast<std::decay_t<LHS>>(lhs symbol make<typename std::remove_reference_t<LHS>::overflow_policy>(std::forward<RHS>(rhs))) \
) \

// bounded::integer being assigned to
BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(+)
BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(-)
BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(*)
BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(/)
BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET(%)

#undef BOUNDED_INTEGER_COMPOUND_ASSIGNMENT_OPERATOR_TARGET

}	// namespace bounded