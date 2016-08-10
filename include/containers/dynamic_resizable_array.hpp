// Copyright David Stone 2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/add_remove_const.hpp>
#include <containers/allocator.hpp>
#include <containers/algorithms/iterator.hpp>
#include <containers/common_container_functions.hpp>
#include <containers/dynamic_array/dynamic_array.hpp>
#include <containers/index_type.hpp>
#include <containers/repeat_n.hpp>
#include <containers/uninitialized_storage.hpp>

#include <bounded/integer_range.hpp>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

namespace containers {
namespace detail {

template<typename Container>
struct dynamic_resizable_array {
	using value_type = typename Container::value_type;
	using allocator_type = typename Container::allocator_type;
	using size_type = typename Container::size_type;

	using const_iterator = detail::basic_array_iterator<value_type const, dynamic_resizable_array>;
	using iterator = detail::basic_array_iterator<value_type, dynamic_resizable_array>;
	
	constexpr decltype(auto) get_allocator() const & noexcept {
		return m_container.get_allocator();
	}
	constexpr decltype(auto) get_allocator() & noexcept {
		return m_container.get_allocator();
	}
	constexpr decltype(auto) get_allocator() && noexcept {
		return std::move(m_container).get_allocator();
	}
	
	constexpr dynamic_resizable_array() noexcept {}
	
	constexpr explicit dynamic_resizable_array(allocator_type allocator) noexcept(std::is_nothrow_constructible<Container, allocator_type &&>::value):
		m_container(std::move(allocator))
	{
	}
	
	template<typename Count, BOUNDED_REQUIRES(std::is_convertible<Count, size_type>::value)>
	constexpr explicit dynamic_resizable_array(Count const count, allocator_type allocator = allocator_type()):
		m_container(std::move(allocator))
	{
		if (count > m_container.capacity()) {
			m_container.relocate(count);
		}
		::containers::uninitialized_default_construct(data(), data() + count, get_allocator());
		m_container.set_size(count);
	}
	template<typename Count, BOUNDED_REQUIRES(std::is_convertible<Count, size_type>::value)>
	constexpr dynamic_resizable_array(Count const count, value_type const & value, allocator_type allocator = allocator_type()):
		m_container(std::move(allocator))
	{
		auto const repeat = detail::repeat_n(count, value);
		assert(size(repeat) == count);
		assign(*this, repeat.begin(), repeat.end());
	}
	
	template<typename ForwardIterator, typename Sentinel, BOUNDED_REQUIRES(is_iterator_sentinel<ForwardIterator, Sentinel>)>
	constexpr dynamic_resizable_array(ForwardIterator first, Sentinel const last, allocator_type allocator = allocator_type()):
		m_container(std::move(allocator))
	{
		assign(*this, first, last);
	}
	
	constexpr dynamic_resizable_array(std::initializer_list<value_type> init, allocator_type allocator = allocator_type()) BOUNDED_NOEXCEPT_INITIALIZATION(
		dynamic_resizable_array(init.begin(), init.end(), std::move(allocator))
	) {}

	constexpr dynamic_resizable_array(dynamic_resizable_array const & other, allocator_type allocator) BOUNDED_NOEXCEPT_INITIALIZATION(
		dynamic_resizable_array(other.begin(), other.end(), std::move(allocator))
	) {}
	constexpr dynamic_resizable_array(dynamic_resizable_array const & other) BOUNDED_NOEXCEPT_INITIALIZATION(
		dynamic_resizable_array(other.begin(), other.end(), other.get_allocator())
	) {}

	constexpr dynamic_resizable_array(dynamic_resizable_array && other, allocator_type allocator) noexcept:
		m_container(std::move(allocator))
	{
		m_container.move_assign_to_empty(std::move(other.m_container));
	}
	constexpr dynamic_resizable_array(dynamic_resizable_array && other) noexcept:
		dynamic_resizable_array(std::move(other), std::move(other).get_allocator())
	{
	}

	constexpr auto & operator=(dynamic_resizable_array const & other) & noexcept(std::is_nothrow_copy_assignable<value_type>::value) {
		assign(*this, other.begin(), other.end());
		return *this;
	}
	constexpr auto & operator=(dynamic_resizable_array && other) & noexcept {
		::containers::detail::destroy(get_allocator(), begin(), end());
		m_container.move_assign_to_empty(std::move(other.m_container));
		return *this;
	}

	constexpr auto & operator=(std::initializer_list<value_type> init) & {
		assign(*this, init.begin(), init.end());
		return *this;
	}

	~dynamic_resizable_array() {
		::containers::detail::destroy(get_allocator(), begin(), end());
	}


	constexpr auto data() const noexcept {
		return m_container.data();
	}
	constexpr auto data() noexcept {
		return m_container.data();
	}

	auto begin() const noexcept {
		return const_iterator(data(), detail::iterator_constructor);
	}
	auto begin() noexcept {
		return iterator(data(), detail::iterator_constructor);
	}
	auto end() const noexcept {
		return begin() + m_container.size();
	}
	auto end() noexcept {
		return begin() + m_container.size();
	}


	auto && operator[](index_type<dynamic_resizable_array> const index) const noexcept {
		assert(index < size(*this));
		return *(begin() + index);
	}
	auto && operator[](index_type<dynamic_resizable_array> const index) noexcept {
		assert(index < size(*this));
		return *(begin() + index);
	}
	
	constexpr auto capacity() const noexcept {
		return m_container.capacity();
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
		if (size(*this) < capacity()) {
			::containers::detail::construct(get_allocator(), data() + size(*this), std::forward<Args>(args)...);
		} else {
			auto temp = m_container.make_storage(new_capacity());
			::containers::detail::construct(get_allocator(), temp.data() + capacity(), std::forward<Args>(args)...);
			::containers::uninitialized_move_destroy(begin(), end(), temp.data(), get_allocator());
			m_container.relocate_preallocated(std::move(temp));
		}
		add_size(1_bi);
	}
	
	// TODO: Remove duplication between emplace and insert
	template<typename... Args>
	auto emplace(const_iterator const position, Args && ... args) {
		check_iterator_validity(position);
		auto const offset = position - begin();
		if (position == end()) {
			emplace_back(std::forward<Args>(args)...);
		} else if (size(*this) != capacity()) {
			detail::emplace_in_middle_no_reallocation(*this, position, get_allocator(), std::forward<Args>(args)...);
		} else {
			// There is a reallocation required, so just put everything in the
			// correct place to begin with
			auto temp = m_container.make_storage(new_capacity());
			// First construct the new element because the arguments to
			// construct it may reference an old element. We cannot move
			// elements it references before constructing it
			auto && allocator = get_allocator();
			::containers::detail::construct(allocator, temp.data() + offset, std::forward<Args>(args)...);
			auto const mutable_position = begin() + offset;
			auto const pointer = ::containers::uninitialized_move_destroy(begin(), mutable_position, temp.data(), allocator);
			assert(temp.data() + offset == pointer);
			::containers::uninitialized_move_destroy(mutable_position, end(), ::containers::next(pointer), allocator);
			m_container.relocate_preallocated(std::move(temp));
			add_size(1_bi);
		}
		return begin() + offset;
	}

	// TODO: Check if the range lies within the container
	// TODO: remove duplication between this and static_vector insert
	template<typename ForwardIterator, typename Sentinel>
	auto insert(const_iterator const position, ForwardIterator first, Sentinel last) {
		check_iterator_validity(position);
		if (position == end()) {
			return append(*this, first, last);
		}

		auto const range_size = bounded::throw_policy<std::out_of_range>{}.assignment(
			::containers::distance(first, last),
			0_bi,
			max_size<dynamic_resizable_array>()
		);
		if (range_size <= capacity()) {
			auto const mutable_position = detail::put_in_middle_no_reallocation(*this, position, first, last, range_size, get_allocator());
			
			add_size(range_size);
			return mutable_position;
		}
		
		// There is a reallocation required, so just put everything in the
		// correct place to begin with
		auto temp = m_container.make_storage(new_capacity());
		auto const offset = position - begin();
		// First construct the new element because it may reference an old
		// element, and we do not want to move elements it references
		::containers::uninitialized_copy(first, last, temp.data() + offset, get_allocator());
		auto const mutable_position = begin() + offset;
		auto const pointer = ::containers::uninitialized_move_destroy(begin(), mutable_position, temp.data(), get_allocator());
		assert(temp.data() + offset == pointer);
		::containers::uninitialized_move_destroy(mutable_position, end(), pointer + range_size, get_allocator());
		m_container.relocate_preallocated(std::move(temp));
		add_size(range_size);
		return mutable_position;
	}


	void pop_back() {
		assert(!empty(*this));
		::containers::detail::destroy(get_allocator(), ::bounded::addressof(back(*this)));
		add_size(-1_bi);
	}
	
private:
	template<typename Size>
	auto add_size(Size const s) BOUNDED_NOEXCEPT_VOID(
		m_container.set_size(m_container.size() + s)
	)
	constexpr auto check_iterator_validity(const_iterator it) {
		assert(it >= begin());
		assert(it <= end());
	}
	constexpr auto new_capacity() const {
		using capacity_type = bounded::equivalent_type<size_type, bounded::throw_policy<>>;
		return bounded::max(1_bi, capacity_type(capacity() * 2_bi));
	}

	Container m_container;
};

}	// namespace detail
}	// namespace containers