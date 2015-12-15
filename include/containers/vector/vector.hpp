// std::vector-like interface around a bounded::array
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

#include <containers/addressof.hpp>
#include <containers/algorithms/iterator.hpp>
#include <containers/common_container_functions.hpp>
#include <containers/dynamic_array/dynamic_array.hpp>
#include <containers/index_type.hpp>
#include <containers/repeat_n.hpp>
#include <containers/uninitialized_storage.hpp>

#include <bounded_integer/integer_range.hpp>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace containers {

template<typename T, typename Allocator = std::allocator<T>>
struct vector {
	using value_type = T;
	using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<value_type>;
	
private:
	using raw_container = dynamic_array<uninitialized_storage<value_type>, allocator_type>;
public:
	using size_type = typename raw_container::size_type;

	using const_iterator = detail::basic_array_iterator<value_type const, vector>;
	using iterator = detail::basic_array_iterator<value_type, vector>;
	
	constexpr decltype(auto) get_allocator() BOUNDED_NOEXCEPT(
		static_cast<allocator_type>(m_container.get_allocator())
	)
	
	constexpr vector() noexcept = default;
	
	constexpr vector(allocator_type allocator):
		m_container(std::move(allocator))
	{
	}
	
	template<typename Count, BOUNDED_REQUIRES(std::is_convertible<Count, size_type>::value)>
	explicit vector(Count const count, allocator_type allocator = allocator_type()):
		m_container(count, std::move(allocator)),
		m_size(capacity())
	{
		::containers::uninitialized_default_construct(begin(), end(), get_allocator());
	}
	template<typename Count, BOUNDED_REQUIRES(std::is_convertible<Count, size_type>::value)>
	vector(Count const count, value_type const & value, allocator_type allocator = allocator_type()):
		m_container(std::move(allocator))
	{
		auto const repeat = detail::repeat_n(count, value);
		assign(*this, repeat.begin(), repeat.end());
	}
	
	template<typename ForwardIterator, typename Sentinel, BOUNDED_REQUIRES(is_iterator<ForwardIterator>)>
	vector(ForwardIterator first, Sentinel const last, allocator_type allocator = allocator_type()):
		m_container(std::move(allocator))
	{
		assign(*this, first, last);
	}
	
	vector(std::initializer_list<value_type> init, allocator_type allocator = allocator_type()) BOUNDED_NOEXCEPT_INITIALIZATION(
		vector(init.begin(), init.end(), std::move(allocator))
	) {}

	vector(vector const & other, allocator_type allocator = allocator_type()) BOUNDED_NOEXCEPT_INITIALIZATION(
		vector(other.begin(), other.end(), std::move(allocator))
	) {}
	vector(vector && other, allocator_type allocator = allocator_type()) noexcept:
		m_container(std::move(other.m_container), std::move(allocator)),
		m_size(std::move(other.m_size))
	{
		other.m_size = bounded::constant<0>;
	}

	auto & operator=(vector const & other) & noexcept(std::is_nothrow_copy_assignable<value_type>::value) {
		assign(*this, other.begin(), other.end());
		return *this;
	}
	auto & operator=(vector && other) & noexcept {
		m_container = std::move(other.m_container);
		m_size = std::move(other.m_size);
		other.m_size = bounded::constant<0>;
		return *this;
	}

	auto & operator=(std::initializer_list<value_type> init) & {
		assign(*this, init.begin(), init.end());
		return *this;
	}

	~vector() {
		// clear leads to stack overflow
		erase(*this, begin(), end());
	}


	auto data() const noexcept {
		return reinterpret_cast<value_type const *>(m_container.data());
	}
	auto data() noexcept {
		return reinterpret_cast<value_type *>(m_container.data());
	}

	auto begin() const noexcept {
		return const_iterator(data());
	}
	auto begin() noexcept {
		return iterator(data());
	}
	auto end() const noexcept {
		return begin() + m_size;
	}
	auto end() noexcept {
		return begin() + m_size;
	}


	auto && operator[](index_type<vector> const index) const noexcept {
		assert(index < size(*this));
		return *(begin() + index);
	}
	auto && operator[](index_type<vector> const index) noexcept {
		assert(index < size(*this));
		return *(begin() + index);
	}
	
	constexpr auto capacity() const noexcept {
		return size(m_container);
	}
	auto reserve(size_type const requested_capacity) {
		if (requested_capacity > capacity()) {
			relocate(requested_capacity);
		}
	}
	auto shrink_to_fit() {
		if (size(*this) != capacity()) {
			relocate(size(*this));
		}
	}

	template<typename... Args>
	auto emplace_back(Args && ... args) {
		auto && allocator = get_allocator();
		if (size(*this) != capacity()) {
			::containers::detail::construct(allocator, data() + size(*this), std::forward<Args>(args)...);
		} else {
			raw_container temp(new_capacity());
			::containers::detail::construct(allocator, temp.data() + capacity(), std::forward<Args>(args)...);
			::containers::uninitialized_move(begin(), end(), temp.begin(), allocator);
			m_container = std::move(temp);
		}
		++m_size;
	}
	
	// TODO: Remove duplication between emplace and insert
	template<typename... Args>
	auto emplace(const_iterator const position, Args && ... args) {
		check_iterator_validity(position);
		auto const offset = position - begin();
		if (position == end()) {
			emplace_back(std::forward<Args>(args)...);
		} else if (size(*this) + bounded::constant<1> != capacity()) {
			detail::emplace_in_middle_no_reallocation(*this, position, get_allocator(), std::forward<Args>(args)...);
		} else {
			// There is a reallocation required, so just put everything in the
			// correct place to begin with
			raw_container temp(new_capacity());
			// First construct the new element because it may reference an old
			// element, and we do not want to move elements it references
			auto && allocator = get_allocator();
			::containers::detail::construct(allocator, temp.data() + offset, std::forward<Args>(args)...);
			auto const mutable_position = begin() + offset;
			auto const pointer = ::containers::uninitialized_move(begin(), mutable_position, temp.data(), allocator);
			assert(temp.data() + offset == pointer);
			::containers::uninitialized_move(mutable_position, end(), ::containers::next(pointer), allocator);
			m_container = std::move(temp);
			++m_size;
		}
		return begin() + offset;
	}

	// TODO: this is supposed to check if the range lies within the vector
	// TODO: remove duplication between this and static_vector insert
	template<typename ForwardIterator, typename Sentinel>
	auto insert(const_iterator const position, ForwardIterator first, Sentinel last) {
		check_iterator_validity(position);
		if (position == end()) {
			return append(*this, first, last);
		}

		auto const range_size = bounded::throw_policy<std::out_of_range>{}.assignment(
			::containers::distance(first, last),
			bounded::constant<0>,
			max_size<vector>()
		);
		if (range_size <= capacity()) {
			auto const mutable_position = detail::put_in_middle_no_reallocation(*this, position, first, last, range_size, get_allocator());
			m_size += range_size;
			return mutable_position;
		}
		
		// There is a reallocation required, so just put everything in the
		// correct place to begin with
		raw_container temp(new_capacity());
		auto const offset = position - begin();
		// First construct the new element because it may reference an old
		// element, and we do not want to move elements it references
		::containers::uninitialized_copy(first, last, temp.data() + offset, get_allocator());
		auto const mutable_position = begin() + offset;
		auto const pointer = ::containers::uninitialized_move(begin(), mutable_position, temp.data(), get_allocator());
		assert(temp.data() + offset == pointer);
		::containers::uninitialized_move(mutable_position, end(), pointer + range_size, get_allocator());
		m_container = std::move(temp);
		m_size += range_size;
		return mutable_position;
	}


	void pop_back() {
		assert(!empty(*this));
		::containers::detail::destroy(get_allocator(), ::containers::addressof(back(*this)));
		--m_size;
	}
	
private:
	constexpr auto check_iterator_validity(const_iterator it) {
		assert(it >= begin());
		assert(it <= end());
	}
	constexpr auto new_capacity() const {
		using capacity_type = bounded::equivalent_type<size_type, bounded::throw_policy<>>;
		return bounded::max(bounded::constant<1>, capacity_type(capacity() * bounded::constant<2>));
	}

	auto relocate(size_type const requested_capacity) {
		raw_container temp(requested_capacity);
		::containers::uninitialized_move(begin(), end(), temp.begin(), get_allocator());
		m_container = std::move(temp);
	}

	raw_container m_container = {};
	size_type m_size = bounded::constant<0>;
};

}	// namespace containers
