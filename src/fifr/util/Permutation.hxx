/*
 * Copyright (c) 2020 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __PERMUTATION_HXX__
#define __PERMUTATION_HXX__

#include "Range.hxx"

#include <algorithm>
#include <array>
#include <numeric>

namespace fifr {
namespace util {

namespace detail {

/// Iterate over subsets of size R.
template <std::size_t R, typename T>
struct nMr {
    nMr(T end) : beg_(), end_(std::move(end)) {}

    nMr(T beg, T end) : beg_(std::move(beg)), end_(std::move(end)) {}

    struct end_iterator {
    };

    struct iterator {
        using pointer = void;
        using value_type = std::array<T, R>;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        iterator(T beg, T end) : beg_(std::move(beg)), end_(std::move(end)) { cur_.fill(beg_); }

        reference operator*() const { return cur_; }

        iterator& operator++()
        {
            for (auto i = R; i > 0;) {
                --i;
                ++cur_[i];
                if (cur_[i] != end_ || i == 0) break;
                cur_[i] = beg_;
            }
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        bool operator==(const iterator& other) const { return cur_ == other.cur_; }

        bool operator!=(const iterator& other) const { return !(*this == other); }

        bool operator<(const iterator& other) const { return cur_ < other.cur_; }

        bool operator<=(const iterator& other) const { return cur_ <= other.cur_; }

        bool operator>=(const iterator& other) const { return cur_ >= other.cur_; }

        bool operator>(const iterator& other) const { return cur_ > other.cur_; }

        bool operator==(const end_iterator&) const { return cur_[0] == end_; }

        bool operator!=(const end_iterator& other) const { return !(*this == other); }

    private:
        std::array<T, R> cur_;
        T beg_;
        T end_;
    };

    iterator begin() const { return {beg_, end_}; }

    end_iterator end() const { return {}; }

private:
    T beg_;
    T end_;
};

/// Iterate over combinations of size R.
template <std::size_t R, typename T>
struct nCr {
    nCr(T end) : beg_(), end_(std::move(end)) {}

    nCr(T beg, T end) : beg_(std::move(beg)), end_(std::move(end)) {}

    struct end_iterator {
    };

    struct iterator {
        using pointer = void;
        using value_type = std::array<T, R>;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        iterator(T beg, T end) : beg_(std::move(beg)), end_(std::move(end))
        {
            if (end_ - beg_ >= static_cast<int>(R)) {
                std::iota(cur_.begin(), cur_.end(), beg_);
            } else {
                std::fill(cur_.begin(), cur_.end(), end_);
            }
        }

        reference operator*() const { return cur_; }

        iterator& operator++()
        {
            for (auto i = R; i > 0;) {
                --i;
                auto nxt = cur_[i];
                ++nxt;
                // current element < next element -> valid
                if ((i == R - 1 && nxt != end_) || (i < R - 1 && nxt != cur_[i + 1])) {
                    cur_[i] = nxt;
                    // start all succeeding elements from their lowest value
                    while (++i < R) {
                        cur_[i] = cur_[i - 1];
                        ++cur_[i];
                    }
                    return *this;
                }
            }
            cur_[0] = end_;
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        bool operator==(const iterator& other) const { return cur_ == other.cur_; }

        bool operator!=(const iterator& other) const { return !(*this == other); }

        bool operator<(const iterator& other) const { return cur_ < other.cur_; }

        bool operator<=(const iterator& other) const { return cur_ <= other.cur_; }

        bool operator>=(const iterator& other) const { return cur_ >= other.cur_; }

        bool operator>(const iterator& other) const { return cur_ > other.cur_; }

        bool operator==(const end_iterator&) const { return cur_[0] == end_; }

        bool operator!=(const end_iterator& other) const { return !(*this == other); }

    private:
        std::array<T, R> cur_;
        T beg_;
        T end_;
    };

    iterator begin() const { return {beg_, end_}; }

    end_iterator end() const { return {}; }

private:
    T beg_;
    T end_;
};

/// Iterate over permutations of size R.
template <std::size_t R, typename T>
struct nPr {
    nPr(T end) : beg_(), end_(std::move(end)) {}

    nPr(T beg, T end) : beg_(std::move(beg)), end_(std::move(end)) {}

    struct end_iterator {
    };

    struct iterator {
        using pointer = void;
        using value_type = std::array<T, R>;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        iterator(T beg, T end) : beg_(std::move(beg)), end_(std::move(end))
        {
            if (end_ - beg_ >= static_cast<int>(R)) {
                std::iota(cur_.begin(), cur_.end(), beg_);
            } else {
                std::fill(cur_.begin(), cur_.end(), end_);
            }
        }

        reference operator*() const { return cur_; }

        iterator& operator++()
        {
            auto i = R - 1;

            ++cur_[i];
            for (;;) {
                if (cur_[i] == end_) {
                    if (i == 0) return *this;
                    cur_[i] = beg_;
                    --i;
                    ++cur_[i];
                } else {
                    bool valid = true;
                    for (auto j = 0ul; valid && j < i; ++j) {
                        valid = cur_[j] != cur_[i];
                    }
                    if (valid) {
                        if (++i == R) return *this;
                    } else {
                        ++cur_[i];
                    }
                }
            }

            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        bool operator==(const iterator& other) const { return cur_ == other.cur_; }

        bool operator!=(const iterator& other) const { return !(*this == other); }

        bool operator<(const iterator& other) const { return cur_ < other.cur_; }

        bool operator<=(const iterator& other) const { return cur_ <= other.cur_; }

        bool operator>=(const iterator& other) const { return cur_ >= other.cur_; }

        bool operator>(const iterator& other) const { return cur_ > other.cur_; }

        bool operator==(const end_iterator&) const { return cur_[0] == end_; }

        bool operator!=(const end_iterator& other) const { return !(*this == other); }

    private:
        std::array<T, R> cur_;
        T beg_;
        T end_;
    };

    iterator begin() const { return {beg_, end_}; }

    end_iterator end() const { return {}; }

private:
    T beg_;
    T end_;
};

template <typename Rng, typename Pred>
struct filtered {
    filtered(Rng rng, Pred pred) : rng_(std::move(rng)), pred_(std::move(pred)) {}

    using end_iterator = typename Rng::end_iterator;

    struct iterator {
        using pointer = void;
        using value_type = typename Rng::iterator::value_type;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        iterator(typename Rng::iterator cur, end_iterator end, Pred pred)
            : cur_(std::move(cur)), end_(std::move(end)), pred_(pred)
        {
            while (cur_ != end_ && !pred_(*cur_)) ++cur_;
        }

        reference operator*() const { return *cur_; }

        iterator& operator++()
        {
            ++cur_;
            while (cur_ != end_ && !pred_(*cur_)) ++cur_;
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        bool operator==(const iterator& other) const { return cur_ == other.cur_; }

        bool operator!=(const iterator& other) const { return !(*this == other); }

        bool operator<(const iterator& other) const { return cur_ < other.cur_; }

        bool operator<=(const iterator& other) const { return cur_ <= other.cur_; }

        bool operator>=(const iterator& other) const { return cur_ >= other.cur_; }

        bool operator>(const iterator& other) const { return cur_ > other.cur_; }

        bool operator==(const end_iterator&) const { return cur_ == end_; }

        bool operator!=(const end_iterator& other) const { return !(*this == other); }

    private:
        typename Rng::iterator cur_;
        end_iterator end_;
        Pred pred_;
    };

    iterator begin() const { return {rng_.begin(), rng_.end(), pred_}; }

    end_iterator end() const { return {}; }

private:
    Rng rng_;
    Pred pred_;
};

}  // namespace detail

template <typename Rng, typename Pred>
auto filtered(Rng rng, Pred pred) -> detail::filtered<Rng, Pred>
{
    return detail::filtered<Rng, Pred>(rng, pred);
}

template <std::size_t R, typename T>
auto nMr(T end) -> detail::nMr<R, T>
{
    return detail::nMr<R, T>(end);
}

template <std::size_t R, typename T>
auto nMr(T beg, T end) -> detail::nMr<R, T>
{
    return detail::nMr<R, T>(beg, end);
}

template <std::size_t R, typename T, typename Pred>
auto nMr(T end, Pred pred) -> detail::filtered<detail::nMr<R, T>, Pred>
{
    return filtered(nMr<R>(std::move(end)), std::move(pred));
}

template <std::size_t R, typename T, typename Pred>
auto nMr(T beg, T end, Pred pred) -> detail::filtered<detail::nMr<R, T>, Pred>
{
    return filtered(nMr<R>(std::move(beg), std::move(end)), std::move(pred));
}

template <std::size_t R, typename T>
auto nCr(T end) -> detail::nCr<R, T>
{
    return detail::nCr<R, T>(end);
}

template <std::size_t R, typename T>
auto nCr(T beg, T end) -> detail::nCr<R, T>
{
    return detail::nCr<R, T>(beg, end);
}

template <std::size_t R, typename T, typename Pred>
auto nCr(T end, Pred pred) -> detail::filtered<detail::nCr<R, T>, Pred>
{
    return filtered(nCr<R>(std::move(end)), std::move(pred));
}

template <std::size_t R, typename T, typename Pred>
auto nCr(T beg, T end, Pred pred) -> detail::filtered<detail::nCr<R, T>, Pred>
{
    return filtered(nCr<R>(std::move(beg), std::move(end)), std::move(pred));
}

template <std::size_t R, typename T>
auto nPr(T end) -> detail::nPr<R, T>
{
    return detail::nPr<R, T>(end);
}

template <std::size_t R, typename T>
auto nPr(T beg, T end) -> detail::nPr<R, T>
{
    return detail::nPr<R, T>(beg, end);
}

template <std::size_t R, typename T, typename Pred>
auto nPr(T end, Pred pred) -> detail::filtered<detail::nPr<R, T>, Pred>
{
    return filtered(nPr<R>(std::move(end)), std::move(pred));
}

template <std::size_t R, typename T, typename Pred>
auto nPr(T beg, T end, Pred pred) -> detail::filtered<detail::nPr<R, T>, Pred>
{
    return filtered(nPr<R>(std::move(beg), std::move(end)), std::move(pred));
}

}  // namespace util
}  // namespace fifr

#endif
