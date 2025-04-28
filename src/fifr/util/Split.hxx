/*
 * Copyright (c) 2016, 2017 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_SPLIT_HXX__
#define __FIFR_UTIL_SPLIT_HXX__

#include "Convert.hxx"
#include "Range.hxx"

#include <array>
#include <iterator>
#include <regex>
#include <string>
#include <utility>

namespace fifr {
namespace util {
/// Split on white spaces by default.
const std::regex default_split = std::regex("[[:space:]]+");

/// Iterator over the parts of a split string.
class SplitIterator : public std::iterator<std::forward_iterator_tag, std::string>
{
    using Super = std::iterator<std::forward_iterator_tag, std::string>;

public:
    using typename Super::difference_type;
    using typename Super::pointer;
    using typename Super::reference;
    using typename Super::value_type;

    SplitIterator() : beg_(std::string::npos), end_(std::string::npos) {}

    SplitIterator(const std::string& str, std::sregex_iterator&& it) : str_(&str), it_(it), beg_(0)
    {
        end_ = it_ != std::sregex_iterator() ? static_cast<std::string::size_type>(it_->position(0)) : std::string::npos;
    }

    std::string operator*() const { return str_->substr(beg_, end_ - beg_); }

    SplitIterator& operator++()
    {
        if (it_ != std::sregex_iterator()) {
            beg_ = end_ + static_cast<std::string::size_type>(it_->length(0));
            ++it_;
            end_ = it_ != std::sregex_iterator() ? static_cast<std::string::size_type>(it_->position(0))
                                                 : std::string::npos;
        } else {
            beg_ = std::string::npos;
            end_ = std::string::npos;
        }
        return *this;
    }

    SplitIterator operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    bool operator==(const SplitIterator& it) const { return beg_ == it.beg_; }

    bool operator!=(const SplitIterator& it) const { return !(*this == it); }

    /// Return the rest of the string starting at the current part.
    [[nodiscard]] std::string rest() const { return str_->substr(beg_); }

private:
    const std::string* str_ = nullptr;
    std::sregex_iterator it_;
    std::string::size_type beg_;
    std::string::size_type end_;
};

/// Return an iterator over parts of a split string.
inline SplitIterator split_begin(const std::string& str, const std::regex& re = default_split)
{
    return {str, std::sregex_iterator(str.begin(), str.end(), re)};
}

/// Return an end iterator over parts of a split string.
inline SplitIterator split_end()
{
    return {};
}

/// Return an iterator proxy of parts of a split string.
inline IteratorProxy<SplitIterator> split(const std::string& str, const std::regex& re = default_split)
{
    return {split_begin(str, re), split_end()};
}

/// Return a split string as a vector.
inline std::vector<std::string> splitv(const std::string& str, const std::regex& re = default_split)
{
    return {split_begin(str, re), split_end()};
}

/// @overload
template <typename T>
std::vector<T> splitv(const std::string& str, const std::regex& re = default_split)
{
    std::vector<T> result;
    for (auto x : split(str, re)) {
        result.push_back(convert<T>(std::move(x)));
    }
    return result;
}

/**
 * Split a string into N parts.
 *
 * The string is split in exactly N parts. The last part contains
 * the rest of the string. If there are too few parts, the remaining
 * parts are empty strings.
 */
template <std::size_t N, typename T = std::string>
std::array<T, N> split(const std::string& str, const std::regex& re = default_split)
{
    std::size_t i = 0;
    std::array<T, N> result;
    auto it = split_begin(str, re);
    auto it_end = split_end();
    while (i < N && it != it_end) {
        result.at(i) = convert<T>(*it);
        ++it;
        ++i;
    }

    return result;
}

/// @internal
namespace detail {
template <typename It>
void split_args(It&, It&)
{
}

template <typename It, typename Arg0, typename... Args>
void split_args(It& it, It& it_end, Arg0& arg0, Args&... args)
{
    if (it != it_end) {
        arg0 = convert<Arg0>(*it);
        split_args(++it, it_end, args...);
    }
}
}  // namespace detail

/**
 * Split a string and cast results to appropriate types.
 *
 * The result variables must implement a `Convert<std::string, T>` trait.
 */
template <typename... Args>
void split(const std::string& str, const std::regex& re, Args&... args)
{
    auto it = split_begin(str, re);
    auto it_end = split_end();
    detail::split_args(it, it_end, args...);
}

namespace detail {
template <typename Tuple, size_t... I>
void split_tuple(const std::string& str, const std::regex& re, Tuple& t, std::index_sequence<I...>)
{
    split(str, re, std::get<I>(t)...);
}
}  // namespace detail

/**
 * Split a string and return casted results as tuple.
 *
 * The result types must implement a `Convert<std::string, T>` trait.
 */
template <typename... Args>
std::tuple<Args...> split(const std::string& str, const std::regex& re = default_split)
{
    static constexpr auto size = std::tuple_size<std::tuple<Args...>>::value;
    std::tuple<Args...> result;
    detail::split_tuple(str, re, result, std::make_index_sequence<size>{});
    return result;
}
}  // namespace util
}  // namespace fifr

#endif
