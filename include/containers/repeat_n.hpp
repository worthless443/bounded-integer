// Copyright David Stone 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/common_iterator_functions.hpp>
#include <containers/operator_bracket.hpp>

#include <bounded/integer.hpp>

#include <iterator>
#include <utility>

namespace containers {
namespace detail {

template<typename Size, typename T>
struct repeat_n_t;

struct repeat_n_sentinel {};

template<typename Size, typename T>
struct repeat_n_iterator {
	using iterator_category = std::random_access_iterator_tag;
	using value_type = std::remove_reference_t<T> const;
	using difference_type = decltype(std::declval<Size>() - std::declval<Size>());
	using pointer = value_type *;
	using reference = value_type &;
	
	constexpr auto const & operator*() const {
		return m_value.get();
	}
	constexpr auto operator->() const {
		return std::addressof(operator*());
	}

	// It is undefined behavior to compare iterators into different ranges
	friend constexpr auto compare(repeat_n_iterator const lhs, repeat_n_iterator const rhs) {
		return compare(lhs.m_remaining, rhs.m_remaining);
	}
	friend constexpr auto compare(repeat_n_iterator const lhs, repeat_n_sentinel) {
		return compare(lhs.m_remaining, 0_bi);
	}
	friend constexpr auto compare(repeat_n_sentinel, repeat_n_iterator const rhs) {
		return compare(0_bi, rhs.m_remaining);
	}

	template<typename Offset, BOUNDED_REQUIRES(std::numeric_limits<Offset>::is_specialized)>
	friend constexpr auto operator+(repeat_n_iterator const it, Offset const offset) {
		return repeat_n_iterator(Size(it.m_remaining - offset), it.m_value);
	}
	friend constexpr auto operator-(repeat_n_iterator const lhs, repeat_n_iterator const rhs) {
		return rhs.m_remaining - lhs.m_remaining;
	}
	friend constexpr auto operator-(repeat_n_sentinel, repeat_n_iterator const rhs) {
		return rhs.m_remaining;
	}

private:
	friend struct repeat_n_t<Size, T>;
	constexpr repeat_n_iterator(Size const remaining, value_type & value):
		m_remaining(remaining),
		m_value(value)
	{
	}

	Size m_remaining;
	std::reference_wrapper<value_type> m_value;
};

template<typename Size, typename T>
struct repeat_n_t {
	using size_type = Size;
	using value_type = T;

	using const_iterator = repeat_n_iterator<size_type, value_type>;
	
	template<typename U>
	repeat_n_t(size_type const size, U && value):
		m_size(size),
		m_value(std::forward<U>(value))
	{
	}

	constexpr auto begin() const noexcept {
		return const_iterator(m_size, m_value);
	}
	constexpr auto end() const noexcept {
		return repeat_n_sentinel{};
	}

	CONTAINERS_OPERATOR_BRACKET_DEFINITIONS(repeat_n_t)

private:
	size_type m_size;
	value_type m_value;
};

template<typename Size, typename T>
constexpr auto repeat_n(Size const size, T && value) {
	using size_type = bounded::integer<0, static_cast<std::intmax_t>(std::numeric_limits<Size>::max())>;
	// T is either an lvalue reference or not a reference
	return repeat_n_t<size_type, T>(size, std::forward<T>(value));
}

}	// namespace detail
}	// namespace containers
