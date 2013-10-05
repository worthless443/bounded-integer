// Ranged integer type tests
// Copyright (C) 2013 David Stone
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

#include "ranged_integer.hpp"
#include "detail/common_type.hpp"
#include "detail/math.hpp"
#include "detail/numeric_limits.hpp"
#include <cassert>
#include <iostream>

namespace {
template<typename integer>
void check_numeric_limits() {
	using int_limits = std::numeric_limits<integer>;
	using ranged_t = checked_integer<int_limits::min(), int_limits::max()>;
	using ranged_limits = std::numeric_limits<ranged_t>;
	static_assert(sizeof(ranged_t) == sizeof(integer), "checked_integer wrong size.");

	// I have to use the preprocessor here to create a string literal
	#define RANGED_INTEGER_CHECK_CONDITION(condition) \
		static_assert(int_limits::condition == ranged_limits::condition, #condition " is wrong.")
	RANGED_INTEGER_CHECK_CONDITION(is_specialized);
	RANGED_INTEGER_CHECK_CONDITION(is_integer);
	RANGED_INTEGER_CHECK_CONDITION(is_exact);
	RANGED_INTEGER_CHECK_CONDITION(has_infinity);
	RANGED_INTEGER_CHECK_CONDITION(has_quiet_NaN);
	RANGED_INTEGER_CHECK_CONDITION(has_signaling_NaN);
	RANGED_INTEGER_CHECK_CONDITION(has_denorm);
	RANGED_INTEGER_CHECK_CONDITION(has_denorm_loss);
	RANGED_INTEGER_CHECK_CONDITION(is_iec559);
	RANGED_INTEGER_CHECK_CONDITION(is_bounded);
	// is_modulo intentionally left out because ranged_integer may differ from
	// the behavior of built-ins. Instead, just instantiate it with a
	// do-nothing test to verify that it compiles.
	static_cast<void>(ranged_limits::is_modulo);
	// RANGED_INTEGER_CHECK_CONDITION(is_modulo);
	RANGED_INTEGER_CHECK_CONDITION(radix);
	RANGED_INTEGER_CHECK_CONDITION(digits);
	RANGED_INTEGER_CHECK_CONDITION(digits10);
	RANGED_INTEGER_CHECK_CONDITION(max_digits10);
	RANGED_INTEGER_CHECK_CONDITION(min_exponent);
	RANGED_INTEGER_CHECK_CONDITION(min_exponent10);
	RANGED_INTEGER_CHECK_CONDITION(max_exponent);
	RANGED_INTEGER_CHECK_CONDITION(max_exponent10);
	RANGED_INTEGER_CHECK_CONDITION(traps);
	RANGED_INTEGER_CHECK_CONDITION(tinyness_before);
	#undef RANGED_INTEGER_CHECK_CONDITION

	#define RANGED_INTEGER_CHECK_FUNCTION(function) \
		static_assert(int_limits::function() == ranged_limits::function(), #function "() is wrong.")
	// Some of the functions are meaningless for integers, so I do not compare
	#define RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(function) \
		static_cast<void>(ranged_limits::function())
	RANGED_INTEGER_CHECK_FUNCTION(min);
	RANGED_INTEGER_CHECK_FUNCTION(lowest);
	RANGED_INTEGER_CHECK_FUNCTION(max);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(epsilon);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(round_error);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(infinity);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(quiet_NaN);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(signaling_NaN);
	RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION(denorm_min);
	#undef RANGED_INTEGER_CHECK_MEANINGLESS_FUNCTION
	#undef RANGED_INTEGER_CHECK_FUNCTION
}
void check_numeric_limits_all() {
	check_numeric_limits<int8_t>();
	check_numeric_limits<uint8_t>();
	check_numeric_limits<int16_t>();
	check_numeric_limits<uint16_t>();
	check_numeric_limits<int32_t>();
	check_numeric_limits<uint32_t>();
	check_numeric_limits<int64_t>();
	// Currently does not support unsigned types equal to uintmax_t
	// check_numeric_limits<uint64_t>();
}

void check_modulo();

void check_arithmetic() {
	constexpr checked_integer<1, 10> const x(9);
	static_assert(sizeof(x) == 1, "checked_integer too big!");
	constexpr checked_integer<-3, 11> const y(x);
	constexpr checked_integer<-3, 11> const z(4);
	static_assert(std::numeric_limits<decltype(z)>::is_signed, "ranged_integer with negative value in range should be signed.");

	constexpr auto sum = x + z;
	static_assert(std::numeric_limits<decltype(sum)>::min() == -2, "Minimum sum incorrect.");
	static_assert(std::numeric_limits<decltype(sum)>::max() == 21, "Maximum sum incorrect.");
	static_assert(sum == 13, "Calculated sum incorrect.");

	constexpr auto difference = x - z;
	static_assert(std::numeric_limits<decltype(difference)>::min() == -10, "Minimum difference incorrect.");
	static_assert(std::numeric_limits<decltype(difference)>::max() == 13, "Maximum difference incorrect.");
	static_assert(difference == 5, "Calculated difference incorrect.");

	constexpr auto product = x * z;
	static_assert(std::numeric_limits<decltype(product)>::min() == -30, "Minimum product incorrect.");
	static_assert(std::numeric_limits<decltype(product)>::max() == 110, "Maximum product incorrect.");
	static_assert(product == 36, "Calculated product incorrect.");

	constexpr auto quotient = x / z;
	static_assert(std::numeric_limits<decltype(quotient)>::min() == -10, "Minimum quotient incorrect.");
	static_assert(std::numeric_limits<decltype(quotient)>::max() == 10, "Maximum quotient incorrect.");
	static_assert(quotient == 2, "Calculated quotient incorrect.");

	constexpr auto negation = -x;
	static_assert(std::numeric_limits<decltype(negation)>::min() == -10, "Minimum quotient incorrect.");
	static_assert(std::numeric_limits<decltype(negation)>::max() == -1, "Maximum quotient incorrect.");
	static_assert(negation == -9, "Calculated negation incorrect.");

	static_assert(quotient < product, "quotient should be less than product.");
	static_assert(difference + 8 == sum, "difference + 8 should equal sum.");
	
	constexpr auto positive = +x;
	static_assert(positive == x, "Unary plus not a no-op.");

	// constexpr checked_integer<2, 8> const z(x);
	// checked_integer<13, 63> const non_overlapping(x);
	
	check_modulo();
}

void check_modulo() {
	constexpr auto ten = make_ranged<10>();
	constexpr auto eleven = make_ranged<11>();
	constexpr auto ten_result = ten % eleven;
	static_assert(ten_result == ten, "Incorrect modulo with divisor one greater");
	static_assert(std::is_same<decltype(ten_result), decltype(ten)>::value, "Incorrect modulo type with divisor one greater");

	constexpr auto nine = make_ranged<9>();
	constexpr auto one = make_ranged<1>();
	constexpr auto one_result = ten % nine;
	static_assert(one_result == one, "Incorrect modulo with divisor one less");
	static_assert(std::is_same<decltype(one_result), decltype(one)>::value, "Incorrect modulo type with divisor one less");

	constexpr auto nine_result = nine % eleven;
	static_assert(nine_result == nine, "Incorrect modulo with divisor two less");
	static_assert(std::is_same<decltype(nine_result), decltype(nine)>::value, "Incorrect modulo type with divisor two less");

	constexpr auto two = make_ranged<2>();
	constexpr auto two_result = eleven % nine;
	static_assert(two_result == two, "Incorrect modulo with divisor two greater");
	static_assert(std::is_same<decltype(two_result), decltype(two)>::value, "Incorrect modulo type with divisor two greater");
	
	
	constexpr checked_integer<17, 23> positive_range(20);
	constexpr checked_integer<-54, -6> negative_range(-33);
	constexpr auto positive_negative_result = positive_range % negative_range;
	constexpr checked_integer<0, 23> positive_negative(20 % -33);
	static_assert(positive_negative_result == positive_negative, "Incorrect modulo with mixed signs");
	static_assert(std::is_same<decltype(positive_negative_result), decltype(positive_negative)>::value, "Incorrect modulo type with mixed signs");
	
	constexpr auto negative_positive_result = negative_range % positive_range;
	constexpr checked_integer<-22, 0> negative_positive(-33 % 20);
	static_assert(negative_positive_result == negative_positive, "Incorrect modulo with mixed signs");
	static_assert(std::is_same<decltype(negative_positive_result), decltype(negative_positive)>::value, "Incorrect modulo type with mixed signs");
	
	constexpr auto result = native_integer<0, 10>(10) % 6;
	static_assert(static_cast<intmax_t>(std::numeric_limits<decltype(result)>::min()) == 0, "uh oh");
	static_assert(static_cast<intmax_t>(std::numeric_limits<decltype(result)>::max()) == 10, "uh oh");
	static_assert(result == 4, "wrong answer");
	
	constexpr auto zero = make_ranged<0>();
	constexpr auto zero_result = zero % 1;
	static_assert(zero_result == zero, "Incorrect modulo with zero for the dividend");
	static_assert(std::is_same<decltype(zero_result), decltype(zero)>::value, "Incorrect modulo type with zero for the dividend");
	// auto undefined = 1 % zero;
}

void check_compound_arithmetic() {
	native_integer<0, 10> x(5);
	x += 5;
	assert(x == 10);
	checked_integer<-10, 10> y(-5);
	y += 5;
	assert(y == 0);
	y += x;	
	assert(y == 10);
	checked_integer<-1000, 1000> z(0);
	z += y;
	assert(z == 10);
	z *= x - 5;
	assert(z == 50);
	z -= 1;
	assert(z == 49);
	z /= 7;
	assert(z == 7);
	z = 0;
	assert(z == 0);
	x %= 6;
	assert(x == 4);
	
	short s = 0;
	s += make_ranged<4>();
	assert(s == 4);
	int i = 9;
	i -= make_ranged(68);
	assert(i == -59);
	long l = -7;
	l *= z;
	assert(l == 0);
	i /= y;
	assert(i == -5);
	i %= make_ranged(6);
	assert(i == -5);
	i %= make_ranged<4>();
	assert(i == -1);

	assert(++x == 5);
	assert(x == 5);
	assert(z++ == 0);
	assert(z == 1);
}

template<typename Initial, intmax_t initial_value, typename Expected, intmax_t expected_value>
void check_absolute_value() {
	constexpr Initial value(initial_value);
	constexpr auto absolute = abs(value);
	constexpr Expected expected_absolute(expected_value);
	static_assert(std::is_same<decltype(expected_absolute), decltype(absolute)>::value, "Absolute value returns the wrong type.");
	static_assert(absolute == expected_absolute, "Absolute value returns the wrong value.");
}

void check_math() {
	check_absolute_value<checked_integer<-10, 4>, -5, checked_integer<0, 10>, 5>();
	check_absolute_value<checked_integer<-10, -10>, -10, checked_integer<10, 10>, 10>();
	check_absolute_value<checked_integer<0, 0>, 0, checked_integer<0, 0>, 0>();
	check_absolute_value<checked_integer<-7, 450>, -1, checked_integer<0, 450>, 1>();
	check_absolute_value<checked_integer<-7, 450>, 1, checked_integer<0, 450>, 1>();
}

void check_common_type() {
	using expected_type = ranged_integer<1, 10, null_policy>;
	using type1 = ranged_integer<1, 5, null_policy>;
	using type2 = ranged_integer<3, 10, null_policy>;
	using common_type = std::common_type<type1, type2>::type;
	static_assert(std::is_same<expected_type, common_type>::value, "common_type wrong.");
}

}	// namespace

int main() {
	check_arithmetic();
	check_compound_arithmetic();
	check_math();
	check_numeric_limits_all();
	check_common_type();
}
