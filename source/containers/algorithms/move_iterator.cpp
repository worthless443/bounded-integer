// Copyright David Stone 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

export module containers.algorithms.move_iterator;

import containers.is_iterator;
import containers.algorithms.transform_iterator;

import bounded;
import std_module;

namespace containers {

template<typename Iterator, typename Base>
concept iterator_base_equality_comparable_with =
	iterator<Iterator> and
	requires(Iterator const it, Base const base) {
		it.base() == base;
	};

template<typename Iterator, typename Base>
concept iterator_base_three_way_comparable_with =
	iterator<Iterator> and
	requires(Iterator const it, Base const base) {
		it.base() <=> base;
	};

template<typename Base>
struct move_iterator_sentinel {
	constexpr explicit move_iterator_sentinel(Base it):
		m_it(std::move(it))
	{
	}
	constexpr auto const & base() const {
		return m_it;
	}
	friend constexpr auto operator<=>(move_iterator_sentinel const & lhs, move_iterator_sentinel const & rhs) = default;
	friend constexpr auto operator<=>(move_iterator_sentinel const & lhs, iterator_base_three_way_comparable_with<Base> auto const & rhs) {
		return lhs.base() <=> rhs.base();
	}
	friend constexpr auto operator==(move_iterator_sentinel const & lhs, move_iterator_sentinel const & rhs) -> bool = default;
	friend constexpr auto operator==(move_iterator_sentinel const & lhs, iterator_base_equality_comparable_with<Base> auto const & rhs) -> bool {
		return lhs.base() == rhs.base();
	}
private:
	Base m_it;
};

export template<typename Iterator>
constexpr auto move_iterator(Iterator it_) {
	if constexpr (iterator<Iterator>) {
		return ::containers::transform_iterator_dereference(std::move(it_), [](Iterator const & it) -> decltype(auto) {
			using base_result = decltype(*it);
			using result = std::conditional_t<
				std::is_reference_v<base_result>,
				std::remove_reference_t<base_result> &&,
				base_result
			>;
			return static_cast<result>(*it);
		});
	} else {
		return move_iterator_sentinel{std::move(it_)};
	}
}

} // namespace containers
