// A vector-like class that can take advantage of cheap moves
// Copyright (C) 2015 David Stone
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

#pragma once

#include <containers/moving_vector/forward_declaration.hpp>

#include <bounded_integer/bounded_integer.hpp>

#include <value_ptr/value_ptr.hpp>

#include <iterator>
#include <type_traits>
#include <vector>

namespace containers {
namespace detail {

using namespace smart_pointer;

template<typename T, typename ValueType, BOUNDED_REQUIRES(std::is_same<T, ValueType>::value)>
constexpr decltype(auto) remove_indirection(value_ptr<T> const * ptr) {
	return **ptr;
}
template<typename T, typename ValueType, BOUNDED_REQUIRES(std::is_same<T, ValueType>::value)>
constexpr decltype(auto) remove_indirection(value_ptr<T> * ptr) {
	return **ptr;
}

template<typename T, typename ValueType, BOUNDED_REQUIRES(std::is_same<value_ptr<T>, ValueType>::value)>
constexpr decltype(auto) remove_indirection(value_ptr<T> const * ptr) {
	return *ptr;
}
template<typename T, typename ValueType, BOUNDED_REQUIRES(std::is_same<value_ptr<T>, ValueType>::value)>
constexpr decltype(auto) remove_indirection(value_ptr<T> * ptr) {
	return *ptr;
}

template<typename T, typename ValueType>
class moving_vector_iterator {
private:
	using indirection_type = value_ptr<T>;
public:
	using value_type = ValueType;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type *;
	using reference = value_type &;
	using iterator_category = std::random_access_iterator_tag;

	constexpr moving_vector_iterator() noexcept = default;
	// Convert iterator or indirect_iterator to const_iterator
	constexpr operator moving_vector_iterator<T, T const>() const noexcept {
		return moving_vector_iterator<T, T const>(m_it);
	}
	// Convert iterator to indirect_iterator.
	constexpr explicit operator moving_vector_iterator<T, indirection_type>() const noexcept {
		return { m_it };
	}

	auto && operator*() const {
		return remove_indirection<std::remove_const_t<T>, std::remove_const_t<value_type>>(m_it);
	}
	auto operator->() const {
		return std::addressof(operator*());
	}

	auto & operator++() {
		++m_it;
		return *this;
	}
	auto operator++(int) {
		auto const self = *this;
		operator++();
		return self;
	}
	auto & operator--() {
		--m_it;
		return *this;
	}
	auto operator--(int) {
		auto const self = *this;
		operator--();
		return self;
	}

	template<typename Offset, BOUNDED_REQUIRES(std::numeric_limits<Offset>::is_integer)>
	friend constexpr auto operator+(moving_vector_iterator const it, Offset const offset) {
		return moving_vector_iterator(it.m_it + offset);
	}
	friend constexpr auto operator-(moving_vector_iterator const lhs, moving_vector_iterator const rhs) {
		return difference_type(lhs.m_it - rhs.m_it);
	}

	template<typename Integer>
	auto & operator[](Integer const index) {
		return *(*this + index);
	}

	friend constexpr auto operator==(moving_vector_iterator const lhs, moving_vector_iterator const rhs) noexcept {
		return lhs.m_it == rhs.m_it;
	}
	friend constexpr auto operator<(moving_vector_iterator const lhs, moving_vector_iterator const rhs) noexcept {
		return lhs.m_it < rhs.m_it;
	}
	
private:
	template<typename U, typename Allocator>
	friend class ::containers::moving_vector;
	template<typename U, typename VT>
	friend class moving_vector_iterator;

	static constexpr auto is_const = std::is_const<value_type>::value;
	using base_iterator = std::conditional_t<is_const, indirection_type const, indirection_type> *;

	constexpr explicit moving_vector_iterator(base_iterator const other) noexcept:
		m_it(other) {
	}
	constexpr explicit moving_vector_iterator(typename std::vector<indirection_type>::iterator const other):
		m_it(std::addressof(*other)) {
	}
	constexpr explicit moving_vector_iterator(typename std::vector<indirection_type>::const_iterator const other):
		m_it(std::addressof(*other)) {
	}

	// Convert const_iterator to iterator
	constexpr explicit operator moving_vector_iterator<T, T>() const noexcept {
		using mutable_type = std::conditional_t<std::is_const<T>::value, indirection_type const, indirection_type> *;
		return moving_vector_iterator<T, T>(const_cast<mutable_type>(m_it));
	}

	base_iterator m_it;
};

template<typename T, typename ValueType>
constexpr auto operator!=(moving_vector_iterator<T, ValueType> const lhs, moving_vector_iterator<T, ValueType> const rhs) noexcept {
	return !(lhs == rhs);
}
template<typename T, typename ValueType>
constexpr auto operator>(moving_vector_iterator<T, ValueType> const lhs, moving_vector_iterator<T, ValueType> const rhs) noexcept {
	return rhs < lhs;
}
template<typename T, typename ValueType>
constexpr auto operator<=(moving_vector_iterator<T, ValueType> const lhs, moving_vector_iterator<T, ValueType> const rhs) noexcept {
	return !(lhs > rhs);
}
template<typename T, typename ValueType>
constexpr auto operator>=(moving_vector_iterator<T, ValueType> const lhs, moving_vector_iterator<T, ValueType> const rhs) noexcept {
	return !(lhs < rhs);
}

template<typename T, typename ValueType>
constexpr auto operator-(moving_vector_iterator<T, ValueType> const lhs, typename moving_vector_iterator<T, ValueType>::difference_type const rhs) {
	return lhs + -rhs;
}
template<typename T, typename ValueType>
constexpr auto operator+=(moving_vector_iterator<T, ValueType> & lhs, typename moving_vector_iterator<T, ValueType>::difference_type const rhs) {
	return lhs = lhs + rhs;
}
template<typename T, typename ValueType>
constexpr auto operator-=(moving_vector_iterator<T, ValueType> & lhs, typename moving_vector_iterator<T, ValueType>::difference_type const rhs) {
	return lhs = lhs - rhs;
}
template<typename T, typename ValueType>
constexpr auto operator+(typename moving_vector_iterator<T, ValueType>::difference_type const lhs, moving_vector_iterator<T, ValueType> rhs) {
	return rhs += lhs;
}

}	// namespace detail
}	// namespace containers