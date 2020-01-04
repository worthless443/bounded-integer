// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <bounded/detail/variant/variant.hpp>

#include <bounded/assert.hpp>
#include <bounded/detail/construct_destroy.hpp>
#include <bounded/forward.hpp>
#include <bounded/is_constructible.hpp>
#include <bounded/tombstone_traits.hpp>
#include <bounded/value_to_function.hpp>

#include <operators/arrow.hpp>

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace bounded {

// none_t cannot be default constructible or we get an ambiguity in op = {};
struct none_t {
	constexpr explicit none_t(int) {}
};
constexpr auto none = none_t(0);

namespace detail {

template<typename T>
struct optional_storage {
	constexpr optional_storage():
		m_data(none_index, none)
	{
	}
	
	template<typename Construct> requires construct_function_for<Construct, T>
	constexpr explicit optional_storage(lazy_init_t, Construct && construct_):
		m_data(lazy_init, value_index, BOUNDED_FORWARD(construct_))
	{
	}
	
	template<typename U> requires is_constructible<T, U &&>
	constexpr explicit optional_storage(U && value):
		m_data(value_index, BOUNDED_FORWARD(value))
	{
	}
	
	constexpr auto is_initialized() const {
		return m_data.index() == value_index;
	}

	constexpr auto uninitialize() {
		::bounded::insert(m_data, none_index, none);
	}

	constexpr auto initialize(construct_function_for<T> auto && construct_) {
		m_data.emplace(value_index, BOUNDED_FORWARD(construct_));
	}
	
	constexpr auto && get() const & {
		return m_data[value_index];
	}
	constexpr auto && get() & {
		return m_data[value_index];
	}
	constexpr auto && get() && {
		return std::move(m_data)[value_index];
	}

private:
	static constexpr auto none_index = constant<0>;
	static constexpr auto value_index = constant<1>;
	variant<none_t, T> m_data;
};

template<typename T>
concept nullable = tombstone_traits<T>::spare_representations != constant<0>;

template<nullable T>
struct optional_storage<T> {
	constexpr optional_storage():
		m_data(make_uninitialized())
	{
	}
	
	template<typename Construct> requires construct_function_for<Construct, T>
	constexpr explicit optional_storage(lazy_init_t, Construct && construct_):
		m_data(lazy_init, BOUNDED_FORWARD(construct_)())
	{
	}
	
	template<typename U> requires is_constructible<T, U &&>
	constexpr explicit optional_storage(U && value):
		m_data(BOUNDED_FORWARD(value))
	{
	}
	
	constexpr auto is_initialized() const {
		return tombstone_traits<T>::index(m_data) != uninitialized_index;
	}

	constexpr auto uninitialize() {
		destroy(m_data);
		construct(lazy_init, m_data, make_uninitialized);
	}

	constexpr auto initialize(construct_function_for<T> auto && construct_) {
		uninitialize();
		construct(lazy_init, m_data, BOUNDED_FORWARD(construct_));
	}

	constexpr auto && get() const & {
		return m_data;
	}
	constexpr auto && get() & {
		return m_data;
	}
	constexpr auto && get() && {
		return std::move(m_data);
	}

private:
	static constexpr auto uninitialized_index = constant<0>;
	static constexpr auto make_uninitialized() noexcept {
		static_assert(noexcept(tombstone_traits<T>::make(uninitialized_index)));
		return tombstone_traits<T>::make(uninitialized_index);
	}
	T m_data;
};

constexpr auto & assign(auto & target, auto && source) {
	if (target) {
		*target = BOUNDED_FORWARD(source);
	} else {
		::bounded::insert(target, BOUNDED_FORWARD(source));
	}
	return target;
}

constexpr auto & assign_from_optional(auto & target, auto && source) {
	if (!source) {
		target = none;
	} else {
		assign(target, *BOUNDED_FORWARD(source));
	}
	return target;
}

} // namespace detail

template<typename T>
struct optional {
private:
	struct common_init_tag{};

public:
	using value_type = T;

	constexpr optional(none_t = none) {
	}

	template<typename Construct> requires construct_function_for<Construct, T>
	constexpr explicit optional(lazy_init_t, Construct && construct_):
		m_storage(lazy_init, BOUNDED_FORWARD(construct_)) {
	}
	template<typename U> requires std::is_convertible_v<U &&, value_type>
	constexpr optional(U && other):
		m_storage(BOUNDED_FORWARD(other))
	{
	}
	template<typename U> requires (!std::is_convertible_v<U &&, value_type> and is_constructible<value_type, U &&>)
	constexpr explicit optional(U && other):
		m_storage(BOUNDED_FORWARD(other))
	{
	}


	template<typename U> requires std::is_convertible_v<U const &, value_type>
	constexpr optional(optional<U> const & other):
		optional(other, common_init_tag{})
	{
	}
	template<typename U> requires std::is_convertible_v<U &&, value_type>
	constexpr optional(optional<U> && other):
		optional(std::move(other), common_init_tag{})
	{
	}
	template<typename U> requires (!std::is_convertible_v<U const &, value_type> and is_constructible<value_type, U const &>)
	constexpr explicit optional(optional<U> const & other):
		optional(other, common_init_tag{})
	{
	}
	template<typename U> requires (!std::is_convertible_v<U &&, value_type> and is_constructible<value_type, U &&>)
	constexpr explicit optional(optional<U> && other):
		optional(std::move(other), common_init_tag{})
	{
	}

	
	constexpr auto operator*() const & -> value_type const & {
		BOUNDED_ASSERT(*this);
		return m_storage.get();
	}
	constexpr auto operator*() & -> value_type & {
		BOUNDED_ASSERT(*this);
		return m_storage.get();
	}
	constexpr auto operator*() && -> value_type && {
		BOUNDED_ASSERT(*this);
		return std::move(m_storage).get();
	}

	OPERATORS_ARROW_DEFINITIONS

	constexpr explicit operator bool() const {
		return m_storage.is_initialized();
	}

	constexpr auto emplace(construct_function_for<T> auto && construct_) {
		m_storage.initialize(BOUNDED_FORWARD(construct_));
	}

	constexpr auto && operator=(none_t) & {
		m_storage.uninitialize();
		return *this;
	}
	// TODO: make this work when value_type is a reference
	template<typename U> requires(std::is_convertible_v<U &&, value_type>)
	constexpr auto && operator=(U && other) & {
		return detail::assign(*this, BOUNDED_FORWARD(other));
	}
	
private:
	constexpr optional(auto && other_optional, common_init_tag):
		optional(none) {
		if (other_optional) {
			::bounded::insert(*this, *BOUNDED_FORWARD(other_optional));
		}
	}
	
	detail::optional_storage<T> m_storage;
};

// A limitation of emulating lazy parameters is that we cannot tell if the user
// is trying to create an optional function or lazily create an optional value
template<typename T> requires(!std::is_same_v<T, none_t> and !std::is_invocable_v<T>)
optional(T) -> optional<T>;

constexpr auto make_optional(auto && value) {
	return optional<std::decay_t<decltype(value)>>(BOUNDED_FORWARD(value));
}

constexpr auto make_optional_lazy(auto && function) -> optional<std::invoke_result_t<decltype(function)>> {
	return {BOUNDED_FORWARD(function)()};
}

} // namespace bounded
namespace std {

template<typename LHS, typename RHS>
struct common_type<bounded::optional<LHS>, RHS> {
	using type = bounded::optional<common_type_t<LHS, RHS>>;
};
template<typename LHS, typename RHS>
struct common_type<LHS, bounded::optional<RHS>> {
	using type = bounded::optional<common_type_t<LHS, RHS>>;
};
template<typename LHS, typename RHS>
struct common_type<bounded::optional<LHS>, bounded::optional<RHS>> {
	using type = bounded::optional<common_type_t<LHS, RHS>>;
};


template<typename RHS>
struct common_type<bounded::none_t, RHS> {
	using type = bounded::optional<RHS>;
};
template<typename LHS>
struct common_type<LHS, bounded::none_t> {
	using type = bounded::optional<LHS>;
};
template<>
struct common_type<bounded::none_t, bounded::none_t> {
	using type = bounded::none_t;
};


template<typename RHS>
struct common_type<bounded::none_t, bounded::optional<RHS>> {
	using type = bounded::optional<RHS>;
};
template<typename LHS>
struct common_type<bounded::optional<LHS>, bounded::none_t> {
	using type = bounded::optional<LHS>;
};

} // namespace std