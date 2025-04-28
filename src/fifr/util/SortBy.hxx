/*
 * Copyright (c) 2016 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_SORTBY_HXX__
#define __FIFR_UTIL_SORTBY_HXX__

/**
 * @file
 *
 * Sort function to sort elements by a key.
 */

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

namespace fifr {
namespace util {
/// Sort container of values by some given container of keys.
template <typename C,
          typename Keys,
          typename std::remove_reference<decltype(std::declval<C>()[0])>::type* = nullptr,
          typename std::remove_reference<decltype(std::declval<C>().size())>::type* = nullptr,
          typename std::remove_reference<decltype(std::declval<Keys>()[0])>::type* = nullptr,
          typename std::remove_reference<decltype(std::declval<Keys>().size())>::type* = nullptr>
void sort_by(C& container, const Keys& keys)
{
    assert(container.size() == keys.size());

    using size_type = typename C::size_type;

    std::vector<size_type> indices;
    indices.reserve(container.size());
    for (size_type i = 0; i < container.size(); i++) {
        indices.push_back(i);
    }

    std::sort(indices.begin(), indices.end(), [&](size_type i, size_type j) { return keys[i] < keys[j]; });

    for (size_type i = 0; i < indices.size(); i++) {
        if (indices[i] != i) {
            auto x = std::move(container[i]);
            auto j = i;
            while (true) {
                auto k = indices[j];
                indices[j] = j;
                if (k != i) {
                    container[j] = std::move(container[k]);
                    j = k;
                } else {
                    container[j] = std::move(x);
                    break;
                }
            }
        }
    }
}

/// Sort container of values by some given function of keys.
template <typename C,
          typename Fun,
          typename std::remove_reference<decltype(std::declval<C>()[0])>::type* = nullptr,
          typename std::remove_reference<decltype(std::declval<C>().size())>::type* = nullptr,
          typename std::remove_reference<decltype(std::declval<Fun>()(std::declval<C>()[0]))>::type* = nullptr>
void sort_by(C& container, Fun fun)
{
    std::vector<decltype(fun(container[0]))> keys;
    keys.reserve(container.size());
    for (auto& x : container) {
        keys.push_back(fun(x));
    }
    sort_by(container, keys);
}
}  // namespace util
}  // namespace fifr

#endif
