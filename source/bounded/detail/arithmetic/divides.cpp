// Copyright David Stone 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <bounded/detail/arithmetic/divides.hpp>
#include <bounded/detail/class.hpp>
#include "extreme_values.hpp"
#include "../../homogeneous_equals.hpp"

namespace {

static_assert(homogeneous_equals(
	bounded::constant<0> / bounded::constant<1>,
	bounded::constant<0>
));
static_assert(homogeneous_equals(
	bounded::constant<1> / bounded::constant<1>,
	bounded::constant<1>
));
static_assert(homogeneous_equals(
	bounded::constant<1> / bounded::constant<2>,
	bounded::constant<0>
));
static_assert(homogeneous_equals(
	bounded::constant<0> / bounded::constant<-1>,
	bounded::constant<0>
));
static_assert(homogeneous_equals(
	bounded::constant<1> / bounded::constant<-1>,
	bounded::constant<-1>
));
static_assert(homogeneous_equals(
	bounded::constant<-1> / bounded::constant<1>,
	bounded::constant<-1>
));
static_assert(homogeneous_equals(
	bounded::constant<-1> / bounded::constant<-1>,
	bounded::constant<1>
));

static_assert(homogeneous_equals(
	signed_max / bounded::constant<1>,
	signed_max
));
static_assert(homogeneous_equals(
	signed_min / bounded::constant<1>,
	signed_min
));
static_assert(homogeneous_equals(
	unsigned_max / bounded::constant<1>,
	unsigned_max
));
static_assert(homogeneous_equals(
	signed_max / bounded::constant<-1>,
	-signed_max
));
#if 0
static_assert(homogeneous_equals(
	signed_min / bounded::constant<-1>,
	-signed_min
));
#endif
static_assert(homogeneous_equals(
	unsigned_max / bounded::constant<2>,
	signed_max
));
static_assert(homogeneous_equals(
	unsigned_max / signed_max,
	bounded::constant<2>
));

static_assert(homogeneous_equals(
	bounded::integer<1, 10>(bounded::constant<9>) / bounded::integer<-3, 11>(bounded::constant<4>),
	bounded::integer<-10, 10>(bounded::constant<2>)
));

// constexpr auto fails_to_compile = bounded::constant<1> / bounded::constant<0>;

}	// namespace
