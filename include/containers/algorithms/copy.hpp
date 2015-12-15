// Algorithms to copy that work with sentinels
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
#include <containers/allocator.hpp>
#include <containers/type_list.hpp>
#include <containers/algorithms/iterator.hpp>

#include <bounded_integer/bounded_integer.hpp>
#include <bounded_integer/integer_range.hpp>

#include <iterator>

namespace containers {
namespace detail {

// reinterpret_cast or static_cast from void * are not allowed in constexpr

template<typename Target, typename Source, typename Enable = void>
struct is_static_castable_c : std::false_type {};

template<typename Target, typename Source>
struct is_static_castable_c<
	Target,
	Source,
	void_t<decltype(static_cast<Target>(std::declval<Source>()))>
> : std::true_type {};

template<typename Target, typename Source>
constexpr auto is_static_castable = is_static_castable_c<Target, Source>::value;

template<typename Target, typename Source, BOUNDED_REQUIRES(is_static_castable<Target, Source>)>
constexpr auto static_or_reinterpret_cast(Source source) noexcept {
	return static_cast<Target>(source);
}

template<typename Target, typename Source, BOUNDED_REQUIRES(!is_static_castable<Target, Source>)>
auto static_or_reinterpret_cast(Source source) noexcept {
	return reinterpret_cast<Target>(source);
}

template<typename Allocator, typename T, typename... Args>
constexpr auto construct(Allocator && allocator, T * pointer, Args && ... args) BOUNDED_NOEXCEPT(
	allocator_traits<std::decay_t<Allocator>>::construct(
		allocator,
		::containers::detail::static_or_reinterpret_cast<typename std::decay_t<Allocator>::value_type *>(pointer),
		std::forward<Args>(args)...
	)
)

template<typename Allocator, typename T>
constexpr auto destroy(Allocator && allocator, T * pointer) BOUNDED_NOEXCEPT(
	allocator_traits<std::decay_t<Allocator>>::destroy(
		allocator,
		::containers::detail::static_or_reinterpret_cast<typename std::decay_t<Allocator>::value_type *>(pointer)
	)
)


template<typename InputIterator, typename Sentinel, typename OutputIterator>
constexpr auto copy(InputIterator first, Sentinel const last, OutputIterator out) {
	struct result {
		InputIterator input;
		OutputIterator output;
	};
	for (; first != last; ++first) {
		*out = *first;
		++out;
	}
	return result { first, out };
}

// Ideally this would be implemented with a counted iterator
template<typename InputIterator, typename Size, typename OutputIterator>
constexpr auto copy_n(InputIterator first, Size const count, OutputIterator out) {
	static_assert(count >= bounded::constant<0>, "Negative numbers not supported.");
	struct result {
		InputIterator input;
		OutputIterator output;
	};
	for (auto const n : bounded::integer_range(count)) {
		static_cast<void>(n);
		*out = *first;
		++out;
		++first;
	}
	return result { first, out };
}


template<
	typename InputIterator, typename Sentinel,
	typename ForwardIterator,
	typename Allocator,
	BOUNDED_REQUIRES(!std::is_nothrow_constructible<
		std::decay_t<decltype(*std::declval<ForwardIterator>())>,
		decltype(*std::declval<InputIterator &>())
	>::value)
>
auto uninitialized_copy(InputIterator first, Sentinel const last, ForwardIterator out, Allocator && allocator) {
	auto out_first = out;
	try {
		for (; first != last; ++first) {
			::containers::detail::construct(allocator, ::containers::addressof(*out), *first);
			++out;
		}
	} catch (...) {
		for (; out_first != out; ++out_first) {
			::containers::detail::destroy(allocator, ::containers::addressof(*out_first));
		}
		throw;
	}
	return out;
}


template<
	typename InputIterator, typename Sentinel,
	typename ForwardIterator,
	typename Allocator,
	BOUNDED_REQUIRES(std::is_nothrow_constructible<
		std::decay_t<decltype(*std::declval<ForwardIterator>())>,
		decltype(*std::declval<InputIterator &>())
	>::value)
>
constexpr auto uninitialized_copy(InputIterator first, Sentinel const last, ForwardIterator out, Allocator && allocator) noexcept {
	for (; first != last; ++first) {
		::containers::detail::construct(allocator, ::containers::addressof(*out), *first);
		++out;
	}
	return out;
}



template<typename InputIterator, typename ForwardIterator, typename Allocator>
constexpr auto uninitialized_move(InputIterator const first, InputIterator const last, ForwardIterator const out, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::detail::uninitialized_copy(std::make_move_iterator(first), std::make_move_iterator(last), out, allocator)
)

template<typename BidirectionalInputIterator, typename BidirectionalOutputIterator, typename Allocator>
constexpr auto uninitialized_copy_backward(BidirectionalInputIterator const first, BidirectionalInputIterator const last, BidirectionalOutputIterator const out_last, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::detail::uninitialized_copy(std::make_reverse_iterator(last), std::make_reverse_iterator(first), std::make_reverse_iterator(out_last), allocator).base()
)

template<typename BidirectionalInputIterator, typename BidirectionalOutputIterator, typename Allocator>
constexpr auto uninitialized_move_backward(BidirectionalInputIterator const first, BidirectionalInputIterator const last, BidirectionalOutputIterator const out_last, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::detail::uninitialized_copy_backward(std::make_move_iterator(first), std::make_move_iterator(last), out_last, allocator)
)


template<
	typename ForwardIterator,
	typename Sentinel,
	typename Allocator,
	BOUNDED_REQUIRES(!std::is_nothrow_default_constructible<std::decay_t<decltype(*std::declval<ForwardIterator const &>())>>::value)
>
auto uninitialized_default_construct(ForwardIterator const first, Sentinel const last, Allocator && allocator) {
	auto it = first;
	try {
		for (; it != last; ++it) {
			::containers::detail::construct(allocator, ::containers::addressof(*it));
		}
	} catch (...) {
		for (auto rollback = first; rollback != it; ++rollback) {
			::containers::detail::destroy(allocator, ::containers::addressof(*rollback));
		}
		throw;
	}
}

template<
	typename ForwardIterator,
	typename Sentinel,
	typename Allocator,
	BOUNDED_REQUIRES(std::is_nothrow_default_constructible<std::decay_t<decltype(*std::declval<ForwardIterator const &>())>>::value)
>
constexpr auto uninitialized_default_construct(ForwardIterator first, Sentinel const last, Allocator && allocator) noexcept {
	for (; first != last; ++first) {
		::containers::detail::construct(allocator, ::containers::addressof(*first));
	}
}

}	// namespace detail
}	// namespace containers
