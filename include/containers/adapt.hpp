// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <containers/iterator_adapter.hpp>
#include <containers/non_modifying_common_container_functions.hpp>
#include <containers/reference_wrapper.hpp>

#include <bounded/integer.hpp>

#include <operators/bracket.hpp>

#include <type_traits>
#include <utility>

namespace containers {

template<typename Range, typename Traits>
struct adapt {
private:
	using adapt_iterator_traits = std::conditional_t<
		std::is_empty_v<Traits> and std::is_copy_assignable_v<Traits>,
		Traits,
		decltype(reference_wrapper(std::declval<Traits const &>()))
	>;

	[[no_unique_address]] Range m_range;
	[[no_unique_address]] Traits m_traits;

public:
	using iterator = decltype(containers::adapt_iterator(
		containers::unwrap(std::declval<adapt_iterator_traits &>()).get_begin(std::declval<Range &>()),
		std::declval<adapt_iterator_traits>()
	));
	using const_iterator = decltype(containers::adapt_iterator(
		containers::unwrap(std::declval<adapt_iterator_traits &>()).get_begin(std::declval<Range const &>()),
		std::declval<adapt_iterator_traits>()
	));
	using value_type = decltype(*std::declval<iterator>());
	using size_type = bounded::integer<
		0,
		bounded::detail::normalize<static_cast<std::uintmax_t>(bounded::max_value<typename std::iterator_traits<iterator>::difference_type>)>
	>;
	
	constexpr adapt(Range && range, Traits traits):
		m_range(BOUNDED_FORWARD(range)),
		m_traits(std::move(traits))
	{
	}
	
	friend constexpr auto begin(adapt const & adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits).get_begin(adapted.m_range),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	friend constexpr auto begin(adapt & adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits.get_begin(adapted.m_range)),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	friend constexpr auto begin(adapt && adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits.get_begin(std::move(adapted).m_range)),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	friend constexpr auto end(adapt const & adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits.get_end(adapted.m_range)),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	friend constexpr auto end(adapt & adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits.get_end(adapted.m_range)),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	friend constexpr auto end(adapt && adapted) {
		return containers::adapt_iterator(
			containers::unwrap(adapted.m_traits.get_end(std::move(adapted).m_range)),
			adapt_iterator_traits(adapted.m_traits)
		);
	}
	
	OPERATORS_BRACKET_SEQUENCE_RANGE_DEFINITIONS
};

template<typename Range, typename Traits>
adapt(Range &&, Traits) -> adapt<Range, Traits>;

}	// namespace containers
