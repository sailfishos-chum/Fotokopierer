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

#ifndef __FIFR_UTIL_SCAN_HXX__
#define __FIFR_UTIL_SCAN_HXX__

#include "AutoTuple.hxx"
#include "Convert.hxx"
#include "Range.hxx"

#include <regex>

namespace fifr {
namespace util {
/// Iterator for matches of a scan operation.
///
/// Elements of this operator are the submatches converted to types
/// Args and returned as a tuple.
template <typename... Args>
class ScanIterator
{
public:
    using difference_type = typename std::sregex_iterator::difference_type;
    using value_type = typename AutoTuple<Args...>::type;
    using pointer = const value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

public:
    ScanIterator(std::sregex_iterator&& it) : it_(it) {}

    value_type operator*() const
    {
        static constexpr auto size = std::tuple_size<std::tuple<Args...>>::value;
        return get_matches(*it_, std::make_index_sequence<size>{});
    }

    ScanIterator& operator++()
    {
        ++it_;
        return *this;
    }

    ScanIterator operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator==(const ScanIterator& other) const { return it_ == other.it_; }

    bool operator!=(const ScanIterator& other) const { return it_ != other.it_; }

private:
    template <size_t... I>
    static value_type get_matches(const std::sregex_iterator::value_type& m, std::index_sequence<I...>)
    {
        return make_auto_tuple(convert<Args>(m.str(I + 1))...);
    }

private:
    std::sregex_iterator it_;
};

/// Full match scan iterator.
///
/// The elements of this iterator are the full scan matches as a string.
template <>
class ScanIterator<>
{
public:
    using difference_type = typename std::sregex_iterator::difference_type;
    using value_type = std::string;
    using pointer = const std::string*;
    using reference = const std::string&;
    using iterator_category = std::forward_iterator_tag;

public:
    ScanIterator(std::sregex_iterator&& it) : it_(it) {}

    value_type operator*() const { return it_->str(); }

    ScanIterator& operator++()
    {
        ++it_;
        return *this;
    }

    ScanIterator operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator==(const ScanIterator& other) const { return it_ == other.it_; }

    bool operator!=(const ScanIterator& other) const { return it_ != other.it_; }

private:
    std::sregex_iterator it_;
};

/**
 * Returns an iterator proxy over all matches within a string.
 *
 * This function should be used within a for loop.
 */
IteratorProxy<std::sregex_iterator> scan_matches(const std::string& str, const std::regex& re);

/**
 * Returns an iterator proxy over all matches within a string.
 *
 * The type of the returned iterator values depends on the number of
 * type arguments:
 *
 * - if the number of type arguments is 0, the returned values are
 *   strings consisting of the whole matches
 * - if the number of type arguments is 1, the returned values are the
 *   first submatches converted to that type
 * - if the number of type arguments is 2, the returned values are
 *   std::pair of of the first two submatches converted to those two
 *   types
 * - if the number of type arguments is greater than two, the returned
 *   values are std::tuple if the submatches converted to the given
 *   types.
 *
 * Note that this function may fail if there are not enough submatches
 * or a conversion fails. So you have to make sure that successful
 * matches can always be converted to the given types.
 *
 * ~~~~~~~~~~~~~~~~~~~~{.cpp}
 * std::string str = "a: 1,2\nb: 3,4\nc: 5,6\n";
 * std::regex re("(\\S+):\\s*(\\d+)\\s*,\\s*(\\d+)");
 *
 * auto r1 = scan(str, re).collect();
 * assert(r1[0] == "a: 1,2");
 * assert(r1[1] == "b: 3,4");
 * assert(r1[2] == "c: 5,6");
 *
 * auto r2 = scan<std::string>(str, re).collect();
 * assert(r2[0] == "a");
 * assert(r2[1] == "b");
 * assert(r2[2] == "c");
 *
 * auto r3 = scan<std::string,int>(str, re).collect();
 * assert(r3[0].first == "a");
 * assert(r3[0].second == 1);
 * assert(r3[1].first == "b");
 * assert(r3[1].second == 3);
 * assert(r3[2].first == "c");
 * assert(r3[2].second == 5);
 *
 * auto r4 = scan<std::string,int,int>(str, re).collect();
 * assert(r4[0] == std::make_tuple("a", 1, 2));
 * assert(r4[1] == std::make_tuple("b", 3, 4));
 * assert(r4[2] == std::make_tuple("c", 5, 6));
 * ~~~~~~~~~~~~~~~~~~~~
 */
template <typename... Args>
IteratorProxy<ScanIterator<Args...>> scan(const std::string& str, const std::regex& re)
{
    return {std::sregex_iterator(str.begin(), str.end(), re), std::sregex_iterator()};
}
}  // namespace util
}  // namespace fifr

#endif
