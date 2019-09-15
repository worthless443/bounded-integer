// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/algorithms/negate.hpp>

#include <bounded/integer.hpp>

#include <type_traits>
#include <utility>

namespace containers {

template<typename InputIterator, typename Sentinel, typename UnaryPredicate>
constexpr auto find_if(InputIterator first, Sentinel const last, UnaryPredicate p) {
	for (; first != last; ++first) {
		if (p(*first)) {
			break;
		}
	}
	return first;
}

template<typename Range, typename UnaryPredicate>
constexpr auto find_if(Range && range, UnaryPredicate p) {
	return ::containers::find_if(begin(BOUNDED_FORWARD(range)), end(BOUNDED_FORWARD(range)), std::move(p));
}


template<typename InputIterator, typename Sentinel, typename UnaryPredicate>
constexpr auto find_if_not(InputIterator const first, Sentinel const last, UnaryPredicate p) {
	return ::containers::find_if(first, last, ::containers::negate(std::move(p)));
}

template<typename Range, typename UnaryPredicate>
constexpr auto find_if_not(Range && range, UnaryPredicate p) {
	return ::containers::find_if_not(begin(BOUNDED_FORWARD(range)), end(BOUNDED_FORWARD(range)), std::move(p));
}


template<typename InputIterator, typename Sentinel, typename T>
constexpr auto find(InputIterator const first, Sentinel const last, T const & value) {
	return ::containers::find_if(first, last, bounded::equal_to(value));
}

template<typename Range, typename T>
constexpr auto find(Range && range, T const & value) {
	return ::containers::find(begin(BOUNDED_FORWARD(range)), end(BOUNDED_FORWARD(range)), value);
}

}	// namespace containers
