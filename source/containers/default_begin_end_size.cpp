// Copyright David Stone 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

module;

#include <operators/forward.hpp>

export module containers.default_begin_end_size;

import containers.begin_end;
import containers.size;

namespace containers {

export struct default_begin_end_size {
	static constexpr auto get_begin(auto && range) {
		return containers::begin(OPERATORS_FORWARD(range));
	}

	static constexpr auto get_end(auto && range) {
		return containers::end(OPERATORS_FORWARD(range));
	}

	static constexpr auto get_size(auto const & range) requires requires { containers::size(range); } {
		return containers::size(range);
	}
	template<typename Range> requires requires { Range::size(); }
	static constexpr auto get_size() {
		return Range::size();
	}
};

} // namespace containers
