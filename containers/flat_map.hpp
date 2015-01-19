// A map-like class that has better locality of reference
// Copyright (C) 2014 David Stone
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

#ifndef CONTAINERS_FLAT_MAP_HPP_
#define CONTAINERS_FLAT_MAP_HPP_

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <value_ptr/enable_if.hpp>
#include "apply_tuple.hpp"
#include "moving_vector.hpp"
#include "unique_inplace_merge.hpp"

namespace containers {
namespace detail_flat_map {
// The exact type of value_type should be considered implementation defined.
// std::pair<key_type const, mapped_type> does not work if the underlying
// container is std::vector because insertion into the middle / sorting requires
// moving the value_type. key_type const does not have a copy or move assignment
// operator. To get the code to work with a std::vector, we have to sacrifice
// some compile-time checks. However, we can leave them in place in the case
// that we are working with a moving_vector.
//
// The allocator should never change the value_type of a container, so we just
// use the default to prevent infinite recursion in our flat_map template
// parameter.
template<typename value_type>
class is_copy_or_move_assignable_helper {
public:
	constexpr bool operator()() const {
		return std::is_move_assignable<value_type>::value or std::is_copy_assignable<value_type>::value;
	}
};
template<typename First, typename Second>
class is_copy_or_move_assignable_helper<std::pair<First, Second>> {
public:
	constexpr bool operator()() const {
		using value_type = std::pair<First, Second>;
		return (std::is_move_assignable<typename value_type::first_type>::value or std::is_copy_assignable<typename value_type::first_type>::value)
			and (std::is_move_assignable<typename value_type::second_type>::value or std::is_copy_assignable<typename value_type::second_type>::value);
	}
};

template<typename Key, typename T, template<typename, typename> class Container>
constexpr bool is_copy_or_move_assignable() {
	using pair_type = std::pair<Key const, T>;
	using container_type = Container<pair_type, std::allocator<pair_type>>;
	using value_ref = decltype(*moving_begin(container_type{}));
	using value_type = typename std::remove_reference<value_ref>::type;
	return is_copy_or_move_assignable_helper<value_type>{}();
}

template<typename Key, typename T, template<typename, typename> class Container>
using value_type_t = std::pair<
	typename std::conditional<is_copy_or_move_assignable<Key, T, Container>(), Key const, Key>::type,
	T
>;

template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
class flat_map_base {
public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = value_type_t<Key, T, Container>;
	using allocator_type = Allocator;
private:
	using container_type = Container<value_type, allocator_type>;
public:
	using size_type = typename container_type::size_type;
	using difference_type = typename container_type::difference_type;
	using const_reference = value_type const &;
	using reference = value_type &;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using key_compare = Compare;
	
	using const_iterator = typename container_type::const_iterator;
	using iterator = typename container_type::iterator;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	
	class value_compare {
	public:
		using result_type = bool;
		using first_argument_type = value_type;
		using second_argument_type = value_type;
		constexpr bool operator()(value_type const & lhs, value_type const & rhs) const {
			return m_compare(lhs.first, rhs.first);
		}
	protected:
		friend class flat_map_base;
		value_compare(key_compare c):
			m_compare(c) {
		}
		key_compare m_compare;
	};
	key_compare key_comp() const {
		return key_compare{};
	}
	value_compare value_comp() const {
		return value_compare(key_comp());
	}
	
	explicit flat_map_base(key_compare const & compare = key_compare{}, Allocator const & allocator = Allocator{}) {
	}
	explicit flat_map_base(Allocator const & allocator) {
	}
	template<typename InputIterator>
	flat_map_base(InputIterator first, InputIterator last, key_compare const & compare = key_compare{}, Allocator const & allocator = Allocator{}):
		m_container(first, last) {
		auto const less = indirect_compare{value_comp()};
		std::sort(moving_begin(m_container), moving_end(m_container), less);
		// At some point this should be std::unique_sort
		auto const equal = [&](auto const & lhs, auto const & rhs) {
			return !less(lhs, rhs) and !less(rhs, lhs);
		};
		m_container.erase(std::unique(moving_begin(m_container), moving_end(m_container), equal), m_container.end());
	}
	template<typename InputIterator>
	flat_map_base(InputIterator first, InputIterator last, Allocator const & allocator):
		flat_map_base(first, last, key_compare{}, allocator) {
	}
	flat_map_base(flat_map_base const & other, Allocator const & allocator):
		flat_map_base(other) {
	}
	flat_map_base(flat_map_base && other, Allocator const & allocator):
		flat_map_base(std::move(other)) {
	}
	flat_map_base(std::initializer_list<value_type> init, key_compare const & compare = key_compare{}, Allocator const & allocator = Allocator{}):
		flat_map_base(std::begin(init), std::end(init), allocator) {
	}
	flat_map_base(std::initializer_list<value_type> init, Allocator const & allocator):
		flat_map_base(init, key_compare{}, allocator) {
	}
	
	flat_map_base & operator=(std::initializer_list<value_type> init) {
		return *this = flat_map_base(init);
	}
	
	const_iterator begin() const noexcept {
		return m_container.begin();
	}
	iterator begin() noexcept {
		return m_container.begin();
	}
	const_iterator cbegin() const noexcept {
		return begin();
	}
	
	const_iterator end() const noexcept {
		return m_container.end();
	}
	iterator end() noexcept {
		return m_container.end();
	}
	const_iterator cend() const noexcept {
		return end();
	}
	
	const_reverse_iterator rbegin() const noexcept {
		return m_container.rbegin();
	}
	reverse_iterator rbegin() noexcept {
		return m_container.rbegin();
	}
	const_reverse_iterator crbegin() const noexcept {
		return rbegin();
	}
	
	const_reverse_iterator rend() const noexcept {
		return m_container.rend();
	}
	reverse_iterator rend() noexcept {
		return m_container.rend();
	}
	const_reverse_iterator crend() const noexcept {
		return rend();
	}
	
	bool empty() const noexcept {
		return m_container.empty();
	}
	size_type size() const noexcept {
		return m_container.size();
	}
	size_type max_size() const noexcept {
		return m_container.max_size();
	}

	// Extra functions on top of the regular map interface
	size_type capacity() const noexcept {
		return m_container.capacity();
	}
	void reserve(size_type const new_capacity) noexcept {
		return m_container.reserve(new_capacity);
	}
	void shink_to_fit() {
		m_container.shrink_to_fit();
	}
	
	void clear() noexcept {
		m_container.clear();
	}
	
	const_iterator lower_bound(key_type const & key) const {
		return std::lower_bound(begin(), end(), key, key_value_compare{key_comp()});
	}
	iterator lower_bound(key_type const & key) {
		return std::lower_bound(begin(), end(), key, key_value_compare{key_comp()});
	}
	const_iterator upper_bound(key_type const & key) const {
		return std::upper_bound(begin(), end(), key, key_value_compare{key_comp()});
	}
	iterator upper_bound(key_type const & key) {
		return std::upper_bound(begin(), end(), key, key_value_compare{key_comp()});
	}
	const_iterator find(key_type const & key) const {
		auto const it = lower_bound(key);
		return (it == end() or key_comp()(key, it->first)) ? end() : it;
	}
	iterator find(key_type const & key) {
		auto const it = lower_bound(key);
		return (it == end() or key_comp()(key, it->first)) ? end() : it;
	}
	
	// Unlike in std::map, insert / emplace can only provide a time complexity
	// that matches an insert into the underlying container, which is to say,
	// linear. An insertion implies shifting all of the elements.
	//
	// Moreover, emplace cannot in general provide the guarantee of no copying
	// or moving. It can only provide the weaker guarantee of no copying or
	// moving of the mapped_type. If the underlying container is a
	// moving_vector, however, then we can perform the operation with no moving
	// of the value_type. In many cases, however, we will have to copy / move
	// the key_type, because we have to construct the key to determine whether
	// we should insert it.
	template<typename... Args>
	std::pair<iterator, bool> emplace(Args && ... args) {
		auto const search_strategy = [&](key_type const & key) {
			return this->upper_bound(key);
		};
		return emplace_search(search_strategy, std::forward<Args>(args)...);
	}
	template<typename... Args>
	iterator emplace_hint(const_iterator hint, Args && ... args) {
		auto const search_strategy = [&](key_type const & key) {
			bool const correct_greater = this->key_comp(key, *hint);
			bool const correct_less = this->key_comp(*std::prev(hint), key);
			bool const correct_hint = correct_greater and correct_less;
			return correct_hint ? hint : this->upper_bound(key);
		};
		return emplace_search(search_strategy, std::forward<Args>(args)...).first;
	}

	template<typename P>
	std::pair<iterator, bool> insert(P && value) {
		return emplace(std::forward<P>(value));
	}
	// This differs from the signature given on cppreference due to ambiguity
	// with the iterator range signature. I'm not sure if this workaround
	// actually fulfills all requirements.
	iterator insert(const_iterator const hint, value_type const & value) {
		return emplace_hint(hint, value);
	}
	iterator insert(const_iterator const hint, value_type && value) {
		return emplace_hint(hint, std::move(value));
	}
	template<typename InputIterator>
	void insert(InputIterator first, InputIterator const last) {
		// Because my underlying container is expected to be contiguous storage,
		// it's best to do a batch insert and then just sort it all. However,
		// because I know that the first section of the final range is already
		// sorted, it's probably better to just sort the new elements then do a
		// merge sort on both ranges, rather than calling std::sort on the
		// entire container.
		//
		// Due to a bug in gcc 4.7's std::vector insert implementation, I cannot
		// simply store the iterator returned by insert (because insert returns
		// void). Instead, I store the original size and construct a new
		// iterator.
		auto const offset = static_cast<typename container_type::difference_type>(m_container.size());
#if 0
		// I rely on the optimizer to remove this static check
		if (allow_duplicates) {
			m_container.insert(m_container.end(), first, last);
		}
		else {
			for (; first != last; ++first) {
				// I feel bad about finding the position just to throw it away, but
				// I suspect this is a win vs. having to shift the elements over.
				auto && value = *first;
				auto const it = find(value.first);
				if (it == end()) {
					m_container.emplace_back(std::forward<decltype(value)>(value));
				}
			}
		}
		auto const midpoint = moving_begin(m_container) + offset;
		indirect_compare const compare(value_comp());
		std::sort(midpoint, moving_end(m_container), compare);
		auto const equal = [&](auto const & lhs, auto const & rhs) {
			return !compare(lhs, rhs) and !compare(rhs, lhs);
		};
		m_container.erase(std::unique(midpoint, moving_end(m_container), equal), m_container.end());
		std::inplace_merge(moving_begin(m_container), midpoint, moving_end(m_container), compare);

#else	// USE_UNIQUE
		m_container.insert(m_container.end(), first, last);
		auto const midpoint = moving_begin(m_container) + offset;
		std::sort(midpoint, moving_end(m_container), indirect_compare{value_comp()});
		if (allow_duplicates) {
			std::inplace_merge(moving_begin(m_container), midpoint, moving_end(m_container), indirect_compare{value_comp()});
		}
		else {
			auto const position = detail::unique_inplace_merge(moving_begin(m_container), midpoint, moving_end(m_container), indirect_compare{value_comp()});
			m_container.erase(position, m_container.end());
		}
#endif
	}
	void insert(std::initializer_list<value_type> init) {
		insert(std::begin(init), std::end(init));
	}
	
	// These maintain the relative order of all elements in the container, so I
	// don't have to worry about re-sorting
	iterator erase(const_iterator const it) {
		return m_container.erase(it);
	}
	iterator erase(const_iterator const first, const_iterator const last) {
		return m_container.erase(first, last);
	}
	
	void swap(flat_map_base & other) noexcept {
		m_container.swap(other.m_container);
	}

	friend bool operator==(flat_map_base const & lhs, flat_map_base const & rhs) noexcept {
		return lhs.m_container == rhs.m_container;
	}
	friend bool operator<(flat_map_base const & lhs, flat_map_base const & rhs) noexcept {
		return lhs.m_container < rhs.m_container;
	}

protected:
	class key_value_compare {
	public:
		constexpr key_value_compare(key_compare const & compare):
			m_compare(compare) {
		}
		constexpr bool operator()(key_type const & key, value_type const & value) const {
			return m_compare(key, value.first);
		}
		constexpr bool operator()(value_type const & value, key_type const & key) const {
			return m_compare(value.first, key);
		}
	private:
		key_compare const & m_compare;
	};
private:
	// It is safe to bind the reference to the object that is being moved in any
	// of these calls to emplace_key because the call to std::move does not
	// actually move anything, it just converts it to an rvalue reference. The
	// object isn't actually moved from until later on in the function.
	//
	// Search represents a function that finds where to insert
	template<typename Search>
	std::pair<iterator, bool> emplace_search(Search const search) {
		// key_type must be default initialized in case it's something like int
		key_type key{};
		return emplace_key(search, key, std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple());
	}
	template<typename Search, typename ConvertToMapped>
	std::pair<iterator, bool> emplace_search(Search const search, key_type const & key, ConvertToMapped && mapped) {
		return emplace_key(search, key, key, std::forward<ConvertToMapped>(mapped));
	}
	template<typename Search, typename ConvertToMapped>
	std::pair<iterator, bool> emplace_search(Search const search, key_type && key, ConvertToMapped && mapped) {
		return emplace_key(search, key, std::move(key), std::forward<ConvertToMapped>(mapped));
	}
	template<typename Search, typename ConvertToKey, typename ConvertToMapped>
	std::pair<iterator, bool> emplace_search(Search const search, ConvertToKey && convert_to_key, ConvertToMapped && mapped) {
		key_type key(convert_to_key);
		return emplace_key(search, key, std::move(key), std::forward<ConvertToMapped>(mapped));
	}
	// These pair constructors could possibly be better written to not construct
	// key twice if ConvertToKey is not the same type as key_type.
	template<typename Search, typename ConvertToKey, typename ConvertToMapped>
	std::pair<iterator, bool> emplace_search(Search const search, std::pair<ConvertToKey, ConvertToMapped> && value) {
		return emplace_key(search, value.first, std::move(value));
	}
	template<typename Search, typename ConvertToKey, typename ConvertToMapped>
	std::pair<iterator, bool> emplace_search(Search const search, std::pair<ConvertToKey, ConvertToMapped> const & value) {
		return emplace_key(search, value.first, value);
	}
	class make_key {
	public:
		template<typename... Args>
		constexpr key_type operator()(Args && ... args) const {
			return key_type(std::forward<Args>(args)...);
		}
	};
	template<typename Search, typename KeyTuple, typename MappedTuple>
	std::pair<iterator, bool> emplace_search(Search const search, std::piecewise_construct_t, KeyTuple && key_tuple, MappedTuple && mapped_tuple) {
		key_type key(apply(make_key{}, std::forward<KeyTuple>(key_tuple)));
		return emplace_key(search, key, std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward<MappedTuple>(mapped_tuple));
	}

	// args contains everything needed to construct value_type, including the
	// key. It is provided here as the first argument just to make things easier
	template<bool duplicates_are_allowed, typename dummy = void>
	class checked_emplace_key;
	template<typename dummy>
	class checked_emplace_key<true, dummy> {
	public:
		using result_type = iterator;
		template<typename Search, typename... Args>
		result_type operator()(flat_map_base & container, Search const search, key_type const & key, Args && ... args) {
			return container.m_container.emplace(search(key), std::forward<Args>(args)...);
		}
	};
	template<typename dummy>
	class checked_emplace_key<false, dummy> {
	public:
		using result_type = std::pair<iterator, bool>;
		template<typename Search, typename... Args>
		result_type operator()(flat_map_base & container, Search const upper_bound, key_type const & key, Args && ... args) {
			auto const position = upper_bound(key);
			// Do not decrement an iterator if it might be begin()
			bool const there_is_element_before = position != container.begin();
			auto const that_element_is_equal = [&](){ return !container.key_comp()(std::prev(position)->first, key); };
			bool const already_exists = there_is_element_before and that_element_is_equal();
			return already_exists ?
				result_type(std::prev(position), false) :
				result_type(container.m_container.emplace(position, std::forward<Args>(args)...), true);
		}
	};
	template<typename Search, typename... Args>
	typename checked_emplace_key<allow_duplicates>::result_type emplace_key(Search const search, key_type const & key, Args && ... args) {
		return checked_emplace_key<allow_duplicates>{}(*this, search, key, std::forward<Args>(args)...);
	}
	
	class indirect_compare {
	public:
		constexpr indirect_compare(value_compare const & compare):
			m_compare(compare) {
		}
		template<typename IndirectionType>
		constexpr bool operator()(IndirectionType const & lhs, IndirectionType const & rhs) const {
			return m_compare(*lhs, *rhs);
		}
		constexpr bool operator()(value_type const & lhs, value_type const & rhs) const {
			return m_compare(lhs, rhs);
		}	
	private:
		value_compare const & m_compare;
	};


	container_type m_container;
};

template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator>
class flat_map : public flat_map_base<Key, T, Compare, Container, Allocator, false> {
private:
	using base = flat_map_base<Key, T, Compare, Container, Allocator, false>;
public:
	using mapped_type = typename base::mapped_type;
	using key_type = typename base::key_type;
	using size_type = typename base::size_type;
	using const_iterator = typename base::const_iterator;
	using iterator = typename base::iterator;

	template<typename ... Args>
	constexpr flat_map(Args && ... args):
		base(std::forward<Args>(args)...) {
	}

	mapped_type const & at(key_type const & key) const {
		auto const it = find(key);
		if (it == this->end()) {
			throw std::out_of_range{"Key not found"};
		}
		return it->second;
	}
	mapped_type & at(key_type const & key) {
		auto const it = this->find(key);
		if (it == this->end()) {
			throw std::out_of_range{"Key not found"};
		}
		return it->second;
	}
	mapped_type & operator[](key_type const & key) {
		return this->emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple()).first->second;
	}
	mapped_type & operator[](key_type && key) {
		return this->emplace(std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple()).first->second;
	}

	std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const {
		auto const it = find(key);
		bool const found = it != this->end();
		return std::make_pair(it, found ? std::next(it) : it);
	}
	std::pair<iterator, iterator> equal_range(key_type const & key) {
		auto const it = find(key);
		bool const found = it != this->end();
		return std::make_pair(it, found ? std::next(it) : it);
	}

	size_type count(key_type const & key) const {
		bool const found = this->find(key) != this->end();
		return found ? 1 : 0;
	}

	size_type erase(key_type const & key) {
		auto const it = this->find(key);
		if (it == this->end()) {
			return 0;
		}
		this->erase(it);
		return 1;
	}
};

template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator>
class flat_multimap : public flat_map_base<Key, T, Compare, Container, Allocator, true> {
private:
	using base = flat_map_base<Key, T, Compare, Container, Allocator, true>;
	using key_value_compare = typename base::key_value_compare;
public:
	using key_type = typename base::key_type;
	using size_type = typename base::size_type;
	using const_iterator = typename base::const_iterator;
	using iterator = typename base::iterator;

	template<typename ... Args>
	constexpr flat_multimap(Args && ... args):
		base(std::forward<Args>(args)...) {
	}

	// These implementations work for map or multimap, but I don't expect the
	// compiler to be able to optimize based on the fact that values in flat_map
	// are unique, so I have slightly different versions in flat_map.

	std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const {
		return std::equal_range(this->begin(), this->end(), key, key_value_compare{this->key_comp()});
	}
	std::pair<iterator, iterator> equal_range(key_type const & key) {
		return std::equal_range(this->begin(), this->end(), key, key_value_compare{this->key_comp()});
	}

	size_type count(key_type const & key) const {
		auto const range = this->equal_range(key);
		return static_cast<size_type>(std::distance(range.first, range.second));
	}
	
	size_type erase(key_type const & key) {
		auto const range = this->equal_range(key);
		if (range.first == this->end()) {
			return 0;
		}
		auto const distance = std::distance(range.first, range.second);
		this->erase(range.first, range.second);
		return distance;
	}
};

}	// namespace detail_flat_map

template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
void swap(detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> & lhs, detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> & rhs) noexcept {
	lhs.swap(rhs);
}

template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
bool operator!=(detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & lhs, detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & rhs) noexcept {
	return !(lhs == rhs);
}
template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
bool operator>(detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & lhs, detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & rhs) noexcept {
	return rhs < lhs;
}
template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
bool operator<=(detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & lhs, detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & rhs) noexcept {
	return !(lhs > rhs);
}
template<typename Key, typename T, typename Compare, template<typename, typename> class Container, typename Allocator, bool allow_duplicates>
bool operator>=(detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & lhs, detail_flat_map::flat_map_base<Key, T, Compare, Container, Allocator, allow_duplicates> const & rhs) noexcept {
	return !(lhs < rhs);
}



template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<detail_flat_map::value_type_t<Key, T, std::vector>>>
using unstable_flat_map = detail_flat_map::flat_map<Key, T, Compare, std::vector, Allocator>;

template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<detail_flat_map::value_type_t<Key, T, moving_vector>>>
using stable_flat_map = detail_flat_map::flat_map<Key, T, Compare, moving_vector, Allocator>;


template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<detail_flat_map::value_type_t<Key, T, std::vector>>>
using unstable_flat_multimap = detail_flat_map::flat_multimap<Key, T, Compare, std::vector, Allocator>;

template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<detail_flat_map::value_type_t<Key, T, moving_vector>>>
using stable_flat_multimap = detail_flat_map::flat_multimap<Key, T, Compare, moving_vector, Allocator>;


}	// namespace containers
#endif	// CONTAINERS_FLAT_MAP_HPP_
