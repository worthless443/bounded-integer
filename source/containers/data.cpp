// Copyright David Stone 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

export module containers.data;

import containers.begin_end;
import containers.is_iterator_sentinel;
import containers.is_range;
import containers.iterator_t;
import containers.to_address;

namespace containers {

template<typename Range>
concept has_member_data = requires(Range r) { r.data(); };

template<typename Iterator, typename Sentinel>
concept contiguous_iterator_sentinel = to_addressable<Iterator> and random_access_sentinel_for<Sentinel, Iterator>;

export template<typename T>
concept contiguous_range =
	random_access_range<T> and
	(has_member_data<T> or contiguous_iterator_sentinel<iterator_t<T>, sentinel_t<T>>);

export template<contiguous_range Range>
constexpr auto data(Range && r) {
	if constexpr (has_member_data<Range>) {
		return r.data();
	} else {
		return containers::to_address(containers::begin(r));
	}
}

} // namespace containers
