// Copyright David Stone 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <containers/ordered_associative_container.hpp>

#include <containers/array.hpp>
#include <containers/c_array.hpp>
#include <containers/flat_map.hpp>
#include <containers/string.hpp>
#include <containers/vector.hpp>

#include <array>
#include <map>
#include <unordered_map>
#include <vector>

namespace {

using namespace bounded::literal;

template<typename... Ts>
constexpr auto all_qualifications_are_ordered_associative_helper = (... and containers::ordered_associative_range<Ts>);

template<typename T>
constexpr auto all_qualifications_are_ordered_associative = all_qualifications_are_ordered_associative_helper<T, T const, T &, T const &, T &&>;

template<typename... Ts>
constexpr auto no_qualifications_are_ordered_associative_helper = (... and !containers::ordered_associative_range<Ts>);

template<typename T>
constexpr auto no_qualifications_are_ordered_associative = no_qualifications_are_ordered_associative_helper<T, T const, T &, T const &, T &&>;

static_assert(no_qualifications_are_ordered_associative<int>);
static_assert(no_qualifications_are_ordered_associative<int *>);

static_assert(no_qualifications_are_ordered_associative<containers::c_array<int, 5>>);

static_assert(no_qualifications_are_ordered_associative<containers::array<int, 5_bi>>);
static_assert(no_qualifications_are_ordered_associative<containers::array<int, 0_bi>>);
static_assert(no_qualifications_are_ordered_associative<containers::vector<int>>);
static_assert(no_qualifications_are_ordered_associative<containers::string>);

static_assert(all_qualifications_are_ordered_associative<containers::flat_map<int, int>>);

static_assert(no_qualifications_are_ordered_associative<std::array<int, 5>>);
static_assert(no_qualifications_are_ordered_associative<std::array<int, 0>>);
static_assert(no_qualifications_are_ordered_associative<std::vector<int>>);
static_assert(all_qualifications_are_ordered_associative<std::map<int, int>>);
static_assert(no_qualifications_are_ordered_associative<std::unordered_map<int, int>>);

}	// namespace
