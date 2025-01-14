// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#include <bounded/assert.hpp>

#include <operators/forward.hpp>
#include <operators/returns.hpp>

export module tv.variant;

import tv.insert;
import tv.get_index;
import tv.is_valid_index;
import tv.overload;
import tv.single_element_storage;
import tv.variadic_union;
import tv.variant_index;
import tv.visit;

import bounded;
import bounded.test_int;
import std_module;

using namespace bounded::literal;

namespace tv {

template<typename T>
concept variant_copy_assignable = std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T>;

template<typename T>
concept variant_trivially_copy_assignable = std::is_trivially_copy_constructible_v<T> and std::is_trivially_copy_assignable_v<T>;


template<typename T>
concept variant_move_assignable = std::is_move_constructible_v<T> and std::is_move_assignable_v<T>;

template<typename T>
concept variant_trivially_move_assignable = std::is_trivially_move_constructible_v<T> and std::is_trivially_move_assignable_v<T>;

constexpr auto equality_visitor = []
	<typename LHS, auto lhs_n, typename RHS, auto rhs_n>
	(visitor_parameter<LHS, lhs_n> const lhs, visitor_parameter<RHS, rhs_n> const rhs)
{
	if constexpr (lhs_n == rhs_n) {
		return lhs.value() == rhs.value();
	} else {
		return false;
	}
};

template<typename Function, typename... Ts>
concept unique_construct_function = matches_exactly_one_type<std::invoke_result_t<Function>, Ts...>;

template<typename Construct>
using constructed_type = std::decay_t<std::invoke_result_t<Construct>>;

struct copy_move_tag{};

export template<typename... Ts>
struct variant {
private:
	template<typename Index>
	using type_at = typename decltype(::tv::get_type(Index(), bounded::type<Ts>...))::type;

public:
	constexpr variant(
		bounded::lazy_init_t,
		auto index_,
		bounded::construct_function_for<type_at<decltype(index_)>> auto && construct_
	):
		m_index(::tv::get_index(index_, bounded::type<Ts>...)),
		m_data(::tv::get_index(index_, bounded::type<Ts>...), OPERATORS_FORWARD(construct_))
	{
	}

	constexpr variant(auto index_, bounded::convertible_to<type_at<decltype(index_)>> auto && value):
		variant(
			bounded::lazy_init,
			index_,
			bounded::value_to_function(OPERATORS_FORWARD(value))
		)
	{
	}

	constexpr variant(bounded::lazy_init_t, unique_construct_function<Ts...> auto && construct_):
		variant(
			bounded::lazy_init,
			bounded::type_t<constructed_type<decltype(construct_)>>(),
			OPERATORS_FORWARD(construct_)
		)
	{
	}
	
	constexpr explicit variant(matches_exactly_one_type<Ts...> auto && value):
		variant(
			bounded::lazy_init,
			bounded::value_to_function(OPERATORS_FORWARD(value))
		)
	{
	}
	

	constexpr variant(variant const &) = default;

	constexpr variant(variant const & other) noexcept(
		(... and std::is_nothrow_copy_constructible_v<Ts>)
	) requires((... and std::is_copy_constructible_v<Ts>) and !(... and std::is_trivially_copy_constructible_v<Ts>)):
		variant(other, copy_move_tag{})
	{
	}


	constexpr variant(variant &&) = default;

	constexpr variant(variant && other) noexcept(
		(... and std::is_nothrow_move_constructible_v<Ts>)
	) requires((... and std::is_move_constructible_v<Ts>) and !(... and std::is_trivially_move_constructible_v<Ts>)):
		variant(std::move(other), copy_move_tag{})
	{
	}

	constexpr auto operator=(variant const &) & -> variant & = default;

	constexpr auto operator=(variant const & other) & noexcept(
		(... and std::is_nothrow_copy_assignable_v<Ts>) and
		(... and std::is_nothrow_copy_constructible_v<Ts>)
	) -> variant & requires((... and variant_copy_assignable<Ts>) and !(... and variant_trivially_copy_assignable<Ts>)) {
		assign(other);
		return *this;
	}


	constexpr auto operator=(variant &&) & -> variant & = default;

	constexpr auto operator=(variant && other) & noexcept(
		(... and std::is_nothrow_move_assignable_v<Ts>) and
		(... and std::is_nothrow_move_constructible_v<Ts>)
	) -> variant & requires((... and variant_move_assignable<Ts>) and !(... and variant_trivially_move_assignable<Ts>)) {
		assign(std::move(other));
		return *this;
	}

	template<typename T> requires(
		(0_bi + ... + bounded::constant<std::same_as<std::decay_t<T>, Ts>>) == 1_bi and
		bounded::constructible_from<std::decay_t<T>, T &&> and
		std::is_assignable_v<std::decay_t<T> &, T &&>
	)
	constexpr auto & operator=(T && value) & {
		visit(*this, [&](auto & original) {
			if constexpr (std::same_as<std::decay_t<decltype(original)>, std::decay_t<T>>) {
				original = OPERATORS_FORWARD(value);
			} else {
				this->emplace_impl(
					bounded::type_t<std::decay_t<T>>(),
					bounded::value_to_function(OPERATORS_FORWARD(value)),
					[&] { bounded::destroy(original); }
				);
			}
		});
		return *this;
	}

	~variant() = default;
	constexpr ~variant() requires(... or !std::is_trivially_destructible_v<Ts>) {
		visit(*this, bounded::destroy);
	}

	constexpr auto index() const -> variant_index<Ts...> {
		return m_index;
	}


	constexpr auto const & operator[](unique_type_identifier<Ts...> auto index_) const & {
		return operator_bracket(m_data, index_);
	}
	constexpr auto & operator[](unique_type_identifier<Ts...> auto index_) & {
		return operator_bracket(m_data, index_);
	}
	constexpr auto && operator[](unique_type_identifier<Ts...> auto index_) && {
		return operator_bracket(std::move(m_data), index_);
	}

	constexpr auto & emplace(auto index, bounded::construct_function_for<type_at<decltype(index)>> auto && construct_) & {
		return emplace_impl(index, OPERATORS_FORWARD(construct_), [&] { visit(*this, bounded::destroy); });
	}
	constexpr auto & emplace(unique_construct_function<Ts...> auto && construct_) & {
		return emplace(bounded::type_t<constructed_type<decltype(construct_)>>(), OPERATORS_FORWARD(construct_));
	}
	
	friend constexpr auto operator==(variant const & lhs, variant const & rhs) -> bool
		requires(... and bounded::equality_comparable<Ts>)
	{
		return visit_with_index(lhs, rhs, equality_visitor);
	}

private:
	constexpr auto & emplace_impl(auto index, bounded::construct_function_for<type_at<decltype(index)>> auto && construct_, auto && destroy_active) & {
		using value_t = type_at<decltype(index)>;
		if constexpr (noexcept(value_t(OPERATORS_FORWARD(construct_)()))) {
			destroy_active();
			return replace_active_member(index, OPERATORS_FORWARD(construct_));
		} else {
			// TODO: Add fallback before this one that does trivial default
			// construction on exception
			auto storage = single_element_storage<value_t>{value_t(OPERATORS_FORWARD(construct_)())};
			destroy_active();
			return replace_active_member(index, [&]{ return bounded::relocate(storage.value); });
		}
	}

	// Assumes the old object has already been destroyed
	constexpr auto & replace_active_member(auto const index, auto && construct_) {
		constexpr auto index_value = ::tv::get_index(index, bounded::type<Ts>...);
		m_index = variant_index<Ts...>(index_value);
		auto & data = bounded::construct_at(m_data, [&] { return variadic_union<Ts...>(index_value, OPERATORS_FORWARD(construct_)); });
		return ::tv::get_union_element(data, index_value);
	}

	constexpr variant(auto && other, copy_move_tag):
		m_index(other.m_index),
		m_data(uninitialized_union())
	{
		visit_with_index(
			OPERATORS_FORWARD(other),
			[&](auto parameter) {
				replace_active_member(parameter.index, [&] -> decltype(auto) { return std::move(parameter).value(); });
			}
		);
	}

	constexpr void assign(auto && other) {
		visit_with_index(
			*this,
			OPERATORS_FORWARD(other),
			[&](auto lhs, auto rhs) {
				if constexpr (lhs.index == rhs.index) {
					lhs.value() = OPERATORS_FORWARD(rhs.value());
				} else {
					::tv::insert(*this, rhs.index, OPERATORS_FORWARD(rhs.value()));
					m_index = variant_index<Ts...>(rhs.index);
				}
			}
		);
	}

	static constexpr auto && operator_bracket(auto && data, auto const index_) {
		return ::tv::get_union_element(
			OPERATORS_FORWARD(data),
			::tv::get_index(index_, bounded::type<Ts>...)
		);
	}

	[[no_unique_address]] variant_index<Ts...> m_index;
	[[no_unique_address]] variadic_union<Ts...> m_data;
};

} // namespace tv

namespace {

using empty_variant_t = tv::variant<>;
static_assert(not std::is_default_constructible_v<empty_variant_t>);
static_assert(not bounded::constructible_from<empty_variant_t, bounded::constant_t<0>>);
static_assert(std::is_empty_v<empty_variant_t>);

struct empty {};
static_assert(std::is_empty_v<tv::variant<empty>>);

using thing_t = tv::variant<int, short, long, char, int>;

static_assert(std::is_trivially_copy_constructible_v<thing_t>);
static_assert(std::is_trivially_move_constructible_v<thing_t>);
static_assert(std::is_trivially_copy_assignable_v<thing_t>);
static_assert(std::is_trivially_move_assignable_v<thing_t>);
static_assert(std::is_trivially_destructible_v<thing_t>);

struct non_trivial {
	non_trivial();
	non_trivial(non_trivial &&);
	non_trivial(non_trivial const &);
	auto operator=(non_trivial &&) -> non_trivial &;
	auto operator=(non_trivial const &) -> non_trivial &;
	~non_trivial();
};

static_assert(!std::is_trivially_copy_constructible_v<tv::variant<non_trivial>>);
static_assert(!std::is_trivially_move_constructible_v<tv::variant<non_trivial>>);
static_assert(!std::is_trivially_copy_assignable_v<tv::variant<non_trivial>>);
static_assert(!std::is_trivially_move_assignable_v<tv::variant<non_trivial>>);
static_assert(!std::is_trivially_destructible_v<tv::variant<non_trivial>>);

template<bool expected, typename Index, typename... Ts>
constexpr bool assert_type_index_concepts() {
	static_assert(tv::matches_exactly_one_type<Index, Ts...> == expected);
	static_assert(!tv::variant_integer_index<bounded::type_t<Index>, sizeof...(Ts)>);
	static_assert(tv::unique_type_identifier<bounded::type_t<Index>, Ts...> == expected);
	return true;
}

template<bool expected, auto index, typename... Ts>
constexpr bool assert_integer_index_concepts() {
	static_assert(!tv::matches_exactly_one_type<bounded::constant_t<index>, Ts...>);
	static_assert(tv::variant_integer_index<bounded::constant_t<index>, sizeof...(Ts)> == expected);
	static_assert(tv::unique_type_identifier<bounded::constant_t<index>, Ts...> == expected);
	return true;
}

static_assert(assert_type_index_concepts<true, int, int>());
static_assert(assert_type_index_concepts<true, int, int, short>());
static_assert(assert_type_index_concepts<false, int, int, int>());
static_assert(assert_type_index_concepts<false, int, short>());
static_assert(assert_type_index_concepts<false, int>());

static_assert(assert_integer_index_concepts<true, 0, int>());
static_assert(assert_integer_index_concepts<true, 0, int, short>());
static_assert(assert_integer_index_concepts<true, 1, int, int>());
static_assert(assert_integer_index_concepts<false, 1, int>());
static_assert(assert_integer_index_concepts<false, 0>());

static_assert(!bounded::constructible_from<thing_t, int>);
static_assert(bounded::constructible_from<thing_t, short>);
static_assert(bounded::constructible_from<thing_t, long>);
static_assert(bounded::constructible_from<thing_t, char>);

static_assert(tv::visit_with_index(
	thing_t(0_bi, 0),
	thing_t(0_bi, 0),
	tv::equality_visitor
));

static_assert(thing_t(0_bi, 0) == thing_t(0_bi, 0));
static_assert(thing_t(0_bi, 0) != thing_t(0_bi, 1));
static_assert(thing_t(0_bi, 0) != thing_t(1_bi, static_cast<short>(0)));
static_assert(thing_t(0_bi, 0) != thing_t(4_bi, 0));
static_assert(thing_t(0_bi, 0) != thing_t(4_bi, 1));

static_assert(thing_t(1_bi, static_cast<short>(5)) == thing_t(static_cast<short>(5)));

static_assert(thing_t(0_bi, 0).index() == 0_bi);
static_assert(thing_t(1_bi, static_cast<short>(0)).index() == 1_bi);
static_assert(thing_t(2_bi, 0).index() == 2_bi);
static_assert(thing_t(3_bi, '\0').index() == 3_bi);
static_assert(thing_t(4_bi, 0).index() == 4_bi);

constexpr auto index = 1_bi;
constexpr auto value = static_cast<short>(8);

constexpr auto thing = thing_t(index, value);
using thingy = decltype(thing[index]);

static_assert(std::same_as<thingy, short const &>);

static_assert(thing[index] == value);
static_assert(tv::visit(thing, [](auto x) { return std::same_as<decltype(x), short>; }));

constexpr auto test_assignment_from_variant() {
	auto thing1 = thing_t(index, value);
	thing1 = thing_t(index, value);
	BOUNDED_ASSERT(thing1[index] == value);
	return true;
}
static_assert(test_assignment_from_variant());

constexpr auto test_assignment_from_value() {
	auto thing1 = thing_t(index, value);
	thing1 = -1L;
	BOUNDED_ASSERT(thing1.index() == bounded::type<long>);
	BOUNDED_ASSERT(thing1[bounded::type<long>] == -1L);
	return true;
}
static_assert(test_assignment_from_value());

struct non_copyable {
	constexpr non_copyable() = default;
	constexpr non_copyable(non_copyable const &) = delete;
	constexpr non_copyable(non_copyable &&) = default;
	constexpr non_copyable & operator=(non_copyable const &) = delete;
	constexpr non_copyable & operator=(non_copyable &&) = default;
};

using non_copyable_variant_t = tv::variant<non_copyable>;
static_assert(not std::is_copy_constructible_v<non_copyable_variant_t>);
static_assert(not std::is_copy_assignable_v<non_copyable_variant_t>);
static_assert(std::is_move_constructible_v<non_copyable_variant_t>);
static_assert(std::is_move_assignable_v<non_copyable_variant_t>);
static_assert(std::same_as<decltype(non_copyable_variant_t(bounded::lazy_init, 0_bi, bounded::construct<non_copyable>)[0_bi]), non_copyable &&>);

struct non_comparable {
};

static_assert(!bounded::equality_comparable<non_comparable>);
static_assert(!bounded::equality_comparable<tv::variant<non_comparable>>);
static_assert(!bounded::equality_comparable<tv::variant<int, non_comparable>>);
static_assert(!bounded::equality_comparable<tv::variant<non_comparable, int>>);
static_assert(!bounded::equality_comparable<tv::variant<int, non_comparable, int>>);

constexpr bool test_non_trivial() {
	using non_trivial_variant_t = tv::variant<bounded_test::integer>;
	auto a = non_trivial_variant_t(bounded_test::integer(3));
	BOUNDED_ASSERT(a.index() == 0_bi);
	auto b = a;
	BOUNDED_ASSERT(a[0_bi].value() == 3);
	BOUNDED_ASSERT(a == b);
	return true;
}
static_assert(test_non_trivial());

constexpr auto variant = tv::variant<int, int *>(5);

constexpr auto int_visitor = [](int) {
	return 3;
};

struct special {
	static constexpr auto operator()(int) {
		return 2;
	}
	static constexpr auto operator()(auto) {
		return 1;
	}
};

static_assert(tv::visit(tv::variant<int>(5), int_visitor) == 3);

static_assert(tv::visit(variant, tv::overload(int_visitor, [](int *) { return 0; })) == 3);
static_assert(tv::visit(variant, tv::overload(int_visitor, [](int const *) { return 0; })) == 3);
static_assert(tv::visit(variant, tv::overload([](auto) { return 0; })) == 0);
static_assert(tv::visit(variant, special{}) == 2);
static_assert(tv::visit(variant, tv::overload(int_visitor, [](auto) { return 0; })) == 3);
static_assert(tv::visit(tv::variant<int>(1), [](int && x) { return x; }) == 1);

static_assert(!std::invocable<decltype(tv::visit), decltype(variant), decltype(int_visitor)>);
static_assert(!std::invocable<decltype(tv::visit), decltype(variant), decltype(tv::overload(int_visitor))>);

static_assert(std::same_as<decltype(bounded::declval<tv::variant<int> &>().emplace(0_bi, [] { return 0; })), int &>);

} // namespace
