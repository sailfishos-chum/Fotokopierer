/*
 * Copyright (c) 2017 Frank Fischer <frank-fischer@shadow-soft.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see  <http://www.gnu.org/licenses/>
 */

#ifndef __FIFR_UTIL_AUTOTUPLE_HXX__
#define __FIFR_UTIL_AUTOTUPLE_HXX__

#include <tuple>

namespace fifr {
namespace util {
/// Trait for returning tuple, pair or value depending on the number
/// of type arguments.
///
/// If the number of type arguments is 1, the returned type is the type itself.
/// If the number of type arguments is 2, the returned type is a pair of these types.
/// Otherwise the returned type is a tuple of these types.
template <typename... Args>
struct AutoTuple {
    using type = std::tuple<Args...>;
};

template <typename Arg>
struct AutoTuple<Arg> {
    using type = Arg;
};

template <typename Arg1, typename Arg2>
struct AutoTuple<Arg1, Arg2> {
    using type = std::pair<Arg1, Arg2>;
};

/// Return the arguments as value, pair or tuple.
///
/// The type of the return value depends on the number of type arguments:
///
/// - if one return the plain argument
/// - if two return a std::pair
/// - otherwise return a std::tuple
template <typename... Args>
typename AutoTuple<Args...>::type make_auto_tuple(Args&&... args)
{
    return typename AutoTuple<Args...>::type{std::forward<Args>(args)...};
}
}  // namespace util
}  // namespace fifr

#endif
