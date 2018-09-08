// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/algorithms/move_destroy_iterator.hpp>
#include <containers/algorithms/move_iterator.hpp>
#include <containers/algorithms/reverse_iterator.hpp>
#include <containers/allocator.hpp>
#include <containers/is_iterator_sentinel.hpp>
#include <containers/scope_guard.hpp>
#include <containers/type.hpp>

#include <bounded/detail/forward.hpp>
#include <bounded/integer.hpp>

#include <iterator>
#include <type_traits>

namespace containers {
namespace detail {

// reinterpret_cast or static_cast from void * are not allowed in constexpr

template<typename Target, typename Source, typename Enable = void>
constexpr auto is_static_castable = false;

template<typename Target, typename Source>
constexpr auto is_static_castable<
	Target,
	Source,
	std::void_t<decltype(static_cast<Target>(std::declval<Source>()))>
> = true;

template<typename Target, typename Source>
constexpr auto static_or_reinterpret_cast(Source source) noexcept {
	if constexpr (is_static_castable<Target, Source>) {
		return static_cast<Target>(source);
	} else {
		return reinterpret_cast<Target>(source);
	}
}

template<typename Allocator, typename T, typename... Args>
constexpr auto construct(Allocator && allocator, T * pointer, Args && ... args) BOUNDED_NOEXCEPT_REF(
	allocator_traits<Allocator>::construct(
		allocator,
		::containers::detail::static_or_reinterpret_cast<typename std::decay_t<Allocator>::value_type *>(pointer),
		BOUNDED_FORWARD(args)...
	)
)

template<typename Allocator, typename T>
constexpr auto destroy(Allocator && allocator, T * pointer) BOUNDED_NOEXCEPT(
	allocator_traits<Allocator>::destroy(
		allocator,
		::containers::detail::static_or_reinterpret_cast<typename std::decay_t<Allocator>::value_type *>(pointer)
	)
)

template<typename Allocator, typename InputIterator, typename Sentinel, BOUNDED_REQUIRES(is_iterator_sentinel<InputIterator, Sentinel>)>
constexpr auto destroy(Allocator && allocator, InputIterator first, Sentinel const last) noexcept {
	// This static_assert fails with reverse_iterator because std::prev is not
	// noexcept
	// static_assert(noexcept(::containers::detail::destroy(allocator, std::addressof(*first))));
	for (; first != last; ++first) {
		::containers::detail::destroy(allocator, std::addressof(*first));
	}
}

}	// namespace detail


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
			::containers::detail::construct(allocator, std::addressof(*out), *first);
			++out;
		}
	} catch (...) {
		::containers::detail::destroy(allocator, out_first, out);
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
		::containers::detail::construct(allocator, std::addressof(*out), *first);
		++out;
	}
	return out;
}



template<typename InputIterator, typename Sentinel, typename ForwardIterator, typename Allocator>
constexpr auto uninitialized_move(InputIterator const first, Sentinel const last, ForwardIterator const out, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::uninitialized_copy(::containers::move_iterator(first), ::containers::move_iterator(last), out, allocator)
)

template<typename BidirectionalInputIterator, typename BidirectionalOutputIterator, typename Allocator>
constexpr auto uninitialized_copy_backward(BidirectionalInputIterator const first, BidirectionalInputIterator const last, BidirectionalOutputIterator const out_last, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::uninitialized_copy(::containers::reverse_iterator(last), ::containers::reverse_iterator(first), ::containers::reverse_iterator(out_last), allocator).base()
)

template<typename BidirectionalInputIterator, typename BidirectionalOutputIterator, typename Allocator>
constexpr auto uninitialized_move_backward(BidirectionalInputIterator const first, BidirectionalInputIterator const last, BidirectionalOutputIterator const out_last, Allocator && allocator) BOUNDED_NOEXCEPT_VALUE(
	::containers::uninitialized_copy_backward(::containers::move_iterator(first), ::containers::move_iterator(last), out_last, allocator)
)


// uninitialized_move_destroy guarantees that all elements in the input range
// have been destoyed when this function completes.
template<
	typename InputIterator,
	typename Sentinel,
	typename ForwardIterator,
	typename Allocator,
	BOUNDED_REQUIRES(not noexcept(::containers::uninitialized_copy(
		::containers::move_destroy_iterator(std::declval<InputIterator const &>()),
		::containers::move_destroy_iterator(std::declval<Sentinel const &>()),
		std::declval<ForwardIterator const &>(),
		std::declval<Allocator &>()
	)))
>
auto uninitialized_move_destroy(InputIterator const first, Sentinel const last, ForwardIterator out, Allocator && allocator) {
	auto first_adapted = ::containers::move_destroy_iterator(first);
	auto const last_adapted = containers::move_destroy_iterator(last);
	auto out_first = out;
	try {
		for (; first_adapted != last_adapted; ++first_adapted) {
			::containers::detail::construct(allocator, std::addressof(*out), *first_adapted);
			++out;
		}
	} catch (...) {
		::containers::detail::destroy(allocator, first_adapted.base(), last);
		::containers::detail::destroy(allocator, out_first, out);
		throw;
	}
	return out;
}

template<
	typename InputIterator,
	typename Sentinel,
	typename ForwardIterator,
	typename Allocator,
	BOUNDED_REQUIRES(noexcept(::containers::uninitialized_copy(
		::containers::move_destroy_iterator(std::declval<InputIterator const &>()),
		::containers::move_destroy_iterator(std::declval<Sentinel const &>()),
		std::declval<ForwardIterator const &>(),
		std::declval<Allocator &>()
	)))
>
constexpr auto uninitialized_move_destroy(InputIterator const first, Sentinel const last, ForwardIterator const out, Allocator && allocator) noexcept {
	return ::containers::uninitialized_copy(
		::containers::move_destroy_iterator(first),
		::containers::move_destroy_iterator(last),
		out,
		allocator
	);
}



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
			::containers::detail::construct(allocator, std::addressof(*it));
		}
	} catch (...) {
		::containers::detail::destroy(allocator, first, it);
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
		::containers::detail::construct(allocator, std::addressof(*first));
	}
}

}	// namespace containers
