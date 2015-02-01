// Also known as saturation
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

#ifndef BOUNDED_INTEGER_POLICY_CLAMP_POLICY_HPP_
#define BOUNDED_INTEGER_POLICY_CLAMP_POLICY_HPP_

#include "basic_policy.hpp"
#include "null_policy.hpp"
#include "../is_bounded_integer.hpp"
#include "../make.hpp"
#include "../minmax.hpp"

namespace bounded {
namespace policy_detail {

struct clamp_policy {
	template<typename T, typename Minimum, typename Maximum>
	static constexpr auto assignment(T && value, Minimum && minimum, Maximum && maximum) noexcept {
		using policy = bounded::null_policy;
		return min(max(bounded::make<policy>(std::forward<T>(value)), bounded::make<policy>(std::forward<Minimum>(minimum))), bounded::make<policy>(std::forward<Maximum>(maximum)));
	}

	static constexpr bool is_modulo = false;
	static constexpr bool overflow_is_error = false;
};

}	// namespace policy_detail

using clamp_policy = basic_policy<policy_detail::clamp_policy>;

}	// namespace bounded
#endif	// BOUNDED_INTEGER_POLICY_CLAMP_POLICY_HPP_
