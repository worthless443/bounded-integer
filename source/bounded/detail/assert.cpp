// Copyright David Stone 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#undef NDEBUG
#undef BOUNDED_NDEBUG

#include <bounded/detail/assert.hpp>

static_assert([]{ BOUNDED_ASSERT(true); return true; }());
