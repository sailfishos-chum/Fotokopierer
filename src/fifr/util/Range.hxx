/*
 * Copyright (c) 2015, 2016, 2017, 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_RANGE_HXX__
#define __FIFR_UTIL_RANGE_HXX__

#include "Convert.hxx"

#include <cassert>
#include <iterator>
#include <type_traits>
#include <vector>

namespace fifr {
namespace util {
#if __cpp_deduction_guides

template <typename T, typename S>
struct range {
    range(T begin, T end, S step) : begin_(std::move(begin)), end_(std::move(end)), step_(std::move(step)) {}

    range(T begin, T end) : begin_(std::move(begin)), end_(std::move(end)), step_() {}

    range(T end) : begin_(), end_(std::move(end)), step_() {}

    struct iterator;

    struct end_iterator {
        friend struct iterator;

        end_iterator(T end) : end_(std::move(end)) {}

    private:
        T end_;
    };

    struct iterator {
        using difference_type = decltype(std::declval<T>() - std::declval<T>());
        using pointer = void;
        using value_type = T;
        using reference = const value_type&;
        using iterator_category = std::random_access_iterator_tag;

        iterator(value_type x, S step) : current_(std::move(x)), step_(std::move(step)) {}

        reference operator*() const { return current_; }

        iterator& operator++()
        {
            current_ += step_;
            return *this;
        }

        iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        iterator& operator+=(difference_type n)
        {
            current_ += n * step_;
            return *this;
        }

        iterator operator+(difference_type n) const
        {
            auto copy = *this;
            copy += n;
            return copy;
        }

        friend iterator operator+(difference_type n, const iterator& it) { return it + n; }

        iterator& operator--()
        {
            current_ -= step_;
            return *this;
        }

        iterator operator--(int)
        {
            auto copy = *this;
            --*this;
            return copy;
        }

        iterator& operator-=(difference_type n)
        {
            current_ -= n * step_;
            return *this;
        }

        iterator operator-(difference_type n) const
        {
            auto copy = *this;
            copy -= n;
            return copy;
        }

        difference_type operator-(const iterator& other) const { return current_ - other.current_; }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const iterator& other) const { return current_ == other.current_; }

        bool operator!=(const iterator& other) const { return !(*this == other); }

        bool operator<(const iterator& other) const { return current_ < other.current_; }

        bool operator<=(const iterator& other) const { return current_ <= other.current_; }

        bool operator>=(const iterator& other) const { return current_ >= other.current_; }

        bool operator>(const iterator& other) const { return current_ > other.current_; }

        bool operator==(const end_iterator& other) const { return current_ >= other.end_; }

        bool operator!=(const end_iterator& other) const { return !(*this == other); }

    private:
        value_type current_;
        S step_;
    };

    [[nodiscard]] iterator begin() const { return iterator(begin_, step_); }

    [[nodiscard]] end_iterator end() const { return {end_}; }

private:
    T begin_;
    T end_;
    S step_;
};

struct OneStep {
    template <typename T>
    friend T& operator+=(T& x, OneStep)
    {
        return ++x;
    }
};

template <typename T>
range(T end)->range<T, OneStep>;

template <typename A, typename B>
range(A begin, B end)->range<typename std::common_type<A, B>::type, OneStep>;

template <typename T>
using range_proxy = range<T, OneStep>;

#else  // C++ 14

namespace detail {
template <typename T>
struct range_iter_base : std::iterator<std::random_access_iterator_tag, T, std::ptrdiff_t, const T*, const T&> {
    using super_type = std::iterator<std::random_access_iterator_tag, T, std::ptrdiff_t, const T*, const T&>;

    using typename super_type::difference_type;
    using typename super_type::pointer;
    using typename super_type::reference;

    explicit range_iter_base(T lcurrent) : current(lcurrent) {}

    reference operator*() const { return current; }

    pointer operator->() const { return &current; }

    range_iter_base& operator++()
    {
        ++current;
        return *this;
    }

    range_iter_base operator++(int)
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    range_iter_base& operator+=(difference_type n)
    {
        current += n;
        return *this;
    }

    range_iter_base operator+(difference_type n) const
    {
        auto copy = *this;
        copy += n;
        return copy;
    }

    friend range_iter_base operator+(difference_type n, const range_iter_base& it) { return it + n; }

    range_iter_base& operator--()
    {
        --current;
        return *this;
    }

    range_iter_base operator--(int)
    {
        auto copy = *this;
        --*this;
        return copy;
    }

    range_iter_base& operator-=(difference_type n)
    {
        current -= n;
        return *this;
    }

    range_iter_base operator-(difference_type n) const
    {
        auto copy = *this;
        copy -= n;
        return copy;
    }

    difference_type operator-(const range_iter_base& other) const { return current - other.current; }

    reference operator[](difference_type n) const { return *(*this + n); }

    bool operator==(const range_iter_base& other) const { return current == other.current; }

    bool operator!=(const range_iter_base& other) const { return !(*this == other); }

    bool operator<(const range_iter_base& other) const { return current < other.current; }

    bool operator<=(const range_iter_base& other) const { return current <= other.current; }

    bool operator>=(const range_iter_base& other) const { return current >= other.current; }

    bool operator>(const range_iter_base& other) const { return current > other.current; }

protected:
    T current;
};

}  // namespace detail

template <typename T>
struct range_proxy {
    /// \internal
    struct iter : detail::range_iter_base<T> {
        explicit iter(T lcurrent) : detail::range_iter_base<T>(lcurrent) {}
    };

    /// \internal
    struct step_range_proxy {
        struct iter : detail::range_iter_base<T> {
            iter(T lcurrent, T lstep) : detail::range_iter_base<T>(lcurrent), step(lstep) {}

            using detail::range_iter_base<T>::current;

            iter& operator++()
            {
                current += step;
                return *this;
            }

            iter operator++(int)
            {
                auto copy = *this;
                ++*this;
                return copy;
            }

            // Loses commutativity. Iterator-based ranges are simply broken. :-(
            bool operator==(iter const& other) const
            {
                return step > 0 ? current >= other.current : current < other.current;
            }

            bool operator!=(iter const& other) const { return !(*this == other); }

        private:
            T step;
        };

        step_range_proxy(T lbegin, T lend, T lstep) : begin_(lbegin, lstep), end_(lend, lstep) {}

        iter begin() const { return begin_; }

        iter end() const { return end_; }

    private:
        iter begin_;
        iter end_;
    };

    range_proxy(T lbegin, T lend) : begin_(lbegin), end_(lend) { assert(lbegin <= lend); }

    step_range_proxy step(T lstep) { return {*begin_, *end_, lstep}; }

    iter begin() const { return begin_; }

    iter end() const { return end_; }

private:
    iter begin_;
    iter end_;

public:
    auto size() const -> decltype(end_ - begin_) { return end_ - begin_; }
};

/// Return range over [begin,end).
template <typename T>
range_proxy<T> range(T begin, T end)
{
    return {begin, end};
}

/// Return range over [0,end)
template <typename T>
range_proxy<T> range(T end)
{
    return {0, end};
}

/// Return range over [begin, begin+step, ..., end).
template <typename T>
typename range_proxy<T>::step_range_proxy range(T begin, T end, T step)
{
    return range(begin, end).step(step);
}

#endif

namespace traits {
/// \internal
template <typename C>
struct has_size {
    template <typename T>
    static constexpr auto check(T*) -> typename std::is_integral<decltype(std::declval<T const>().size())>::type
    {
        return std::true_type();
    }

    template <typename>
    static constexpr auto check(...) -> std::false_type;

    using type = decltype(check<C>(nullptr));
    static constexpr bool value = type::value;
};

}  // namespace traits

/**
 * Return range over the valid indices of a container.
 *
 * This requires the container to implement a `size()` method.
 */
template <typename C, typename = typename std::enable_if<traits::has_size<C>::value>>
auto indices(C const& cont) -> range_proxy<decltype(cont.size())>
{
    return {0, cont.size()};
}

/**
 * Return range over the valid indices of a fixed size array.
 */
template <typename T, std::size_t N>
range_proxy<std::size_t> indices(T (&)[N])
{
    return {0, N};
}

/**
 * Return range over the valid indices of an initializer list.
 */
template <typename T>
range_proxy<typename std::initializer_list<T>::size_type> indices(std::initializer_list<T>&& cont)
{
    return {0, cont.size()};
}

/// Range covering all elements.
struct all_range {
};

constexpr all_range all{};

/// Beginning of an all-range.
template <typename Size>
Size range_begin(all_range, Size)
{
    return 0;
}

/// End of an all-range.
template <typename Size>
Size range_end(all_range, Size n)
{
    return n;
}

/// Size of an all-range.
template <typename Size>
Size range_size(all_range, Size n)
{
    return n;
}

/// Begin of a single value range.
template <typename T, typename Size, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
Size range_begin(T i, Size)
{
    return i;
}

/// Begin of a single value range.
template <typename T, typename Size, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
Size range_end(T i, Size)
{
    return i + 1;
}

/// Size of a single value range.
template <typename T, typename Size, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
Size range_size(T, Size)
{
    return 1;
}

/// Begin of a regular index range.
template <typename T, typename Size>
Size range_begin(const range_proxy<T>& rng, Size)
{
    return *rng.begin();
}

/// End of a regular index range.
template <typename T, typename Size>
Size range_end(const range_proxy<T>& rng, Size)
{
    return *rng.end();
}

/// Size of a regular index range.
template <typename T, typename Size>
Size range_size(const range_proxy<T>& rng, Size)
{
    return rng.end() - rng.begin();
}

/// Proxy providing access to some iterators.
template <typename It_>
class IteratorProxy
{
public:
    using iterator = It_;
    using value_type = typename iterator::value_type;

public:
    IteratorProxy(iterator&& start, iterator&& end) : start_(std::move(start)), end_(std::move(end)) {}

    IteratorProxy(const IteratorProxy&) = delete;

    IteratorProxy(IteratorProxy&&) = delete;

    void operator=(const IteratorProxy&) = delete;

    void operator=(IteratorProxy&&) = delete;

    ~IteratorProxy() = default;

    [[nodiscard]] iterator begin() const { return start_; }

    [[nodiscard]] iterator end() const { return end_; }

    template <typename T>
    operator std::vector<T>() const&
    {
        std::vector<T> result;
        for (auto it : *this) {
            result.push_back(convert<T>(it));
        }
        return result;
    }

    template <typename T>
    operator std::vector<T>() &&
    {
        std::vector<T> result;
        for (auto&& val : *this) {
            result.push_back(convert<T>(std::move(val)));
        }
        return result;
    }

    [[nodiscard]] std::vector<value_type> collect() const& { return {start_, end_}; }

    std::vector<value_type> collect() && { return {std::move(start_), std::move(end_)}; }

private:
    iterator start_;
    iterator end_;
};

}  // namespace util
}  // namespace fifr

#endif  // __FIFR_UTIL_RANGE_HXX__
