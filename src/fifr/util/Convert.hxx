/*
 * Copyright (c) 2016-2020 Frank Fischer <frank-fischer@shadow-soft.de>
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

/**
 * @file
 *
 * Implement a Rust inspired conversion trait `Convert`.
 */

#ifndef __FIFR_UTIL_CONVERT_HXX__
#define __FIFR_UTIL_CONVERT_HXX__

#include <algorithm>
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

# ifndef __has_include
#   define __has_include(x) 0
# endif
# if __has_include(<version>)
#   include <version>
# elif __has_include(<optional>)
#   include <optional>
# endif
# if __cpp_lib_optional >= 201606
#   define have_optional 1
# else
#   define have_optional 0
# endif

namespace fifr {
namespace util {
template <typename T, typename F, typename Enable = void>
struct Convert;

/// Convert an element to itself.
///
/// This conversion does nothing.
template <typename T>
struct Convert<T, T> {
#if have_optional
    inline static std::optional<T> to(T&& x) { return std::forward<T>(x); }
#endif

    inline static T convert(T&& x) { return std::forward<T>(x); }
    inline static const T& convert(const T& x) { return x; }
};

/// Converting to a `std::string`.
///
/// This conversion requires `std::to_string` to be implemented for the source
/// type.
template <typename F>
struct Convert<std::string, F, decltype((void)std::to_string(std::declval<F>()))> {
#if have_optional
    inline static std::optional<std::string> to(const F& x) { return std::to_string(x); }
#endif

    inline static std::string convert(const F& x) { return std::to_string(x); }
};

/// Converting to a `std::string` from a `std::string_view`.
#ifdef __cpp_lib_string_view
template <>
struct Convert<std::string, std::string_view> {
#if have_optional
    inline static std::optional<std::string> to(std::string_view x) { return std::string(x); }
#endif

    inline static std::string convert(std::string_view x) { return std::string(x); }
};
#endif

/// Convert a `std::string_view` to a numeric value.
///
/// The conversion requires `std::from_chars`.
template <typename T>
struct Convert<T,
#ifdef __cpp_lib_string_view
               std::string_view,
#else
               std::string,
#endif
               decltype((void)std::from_chars(nullptr, nullptr, std::declval<T&>()))> {

#ifdef __cpp_lib_string_view
    using string_type = std::string_view;
#else
    using string_type = const std::string&;
#endif

#if have_optional
    inline static std::optional<T> to(string_type str)
    {
        T x;
        if (auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), x); ec == std::errc()) {
            if (p == str.data() + str.size()) {
                return x;
            }
        }
        return {};
    }
#endif

    inline static T convert(string_type str)
    {
        T x;
        auto r = std::from_chars(str.data(), str.data() + str.size(), x);
        if (r.ec == std::errc()) {
            if (r.ptr == str.data() + str.size()) {
                return x;
            } else {
                throw std::invalid_argument("convert: invalid argument");
            }
        } else if (r.ec == std::errc::invalid_argument) {
            throw std::invalid_argument("convert: invalid argument");
        } else if (r.ec == std::errc::result_out_of_range) {
            throw std::out_of_range("convert: out of range");
        } else {
            throw std::invalid_argument("convert: invalid argument");
        }
    }
};

/// Convert a `std::string_view` to a float.
template <typename T>
struct Convert<T,
#ifdef __cpp_lib_string_view
               std::string_view,
#else
               std::string,
#endif
               typename std::enable_if<std::is_floating_point<T>::value>::type> {
#ifdef __cpp_lib_string_view
    using string_type = std::string_view;
#else
    using string_type = const std::string&;
#endif

#if have_optional
    inline static std::optional<T> to(string_type str)
    {
        char* end = nullptr;
        T x;
        from_str(str.data(), &end, x);
        if (errno != ERANGE && end != str.data()) {
            return x;
        }
        return {};
    }
#endif

    inline static T convert(string_type str)
    {
        char* end = nullptr;
        T x;
        from_str(str.data(), &end, x);
        if (errno == ERANGE) {
            throw std::out_of_range("convert: out of range");
        }
        if (end == str.data()) {
            throw std::invalid_argument("convert: invalid argument");
        }
        return x;
    }

private:
    static void from_str(const char* str, char** end, float& x) { x = std::strtof(str, end); }

    static void from_str(const char* str, char** end, double& x) { x = std::strtod(str, end); }

    static void from_str(const char* str, char** end, long double& x) { x = std::strtold(str, end); }
};

#ifdef __cpp_lib_string_view
/// Convert a `std::string` to a numeric value.
template <typename T>
struct Convert<T, std::string, typename std::enable_if<std::is_arithmetic<T>::value>::type>
    : public Convert<T, std::string_view> {
};
#endif

/// Convert a `char[]` to a numeric value.
template <typename T, std::size_t N>
struct Convert<T, char[N]> : public Convert<T,
#ifdef __cpp_lib_string_view
                                            std::string_view
#else
                                            std::string
#endif
                                            > {
};

/// Convert a `char*` to a numeric value.
template <typename T>
struct Convert<T, const char*> : public Convert<T,
#ifdef __cpp_lib_string_view
                                                std::string_view
#else
                                                std::string
#endif
                                                > {
};

/// Convert an integral type to an arithmetic with bounds checking.
template <typename T, typename F>
struct Convert<
    T,
    F,
    typename std::enable_if<!std::is_same<T, F>::value && std::is_arithmetic<T>::value && std::is_integral<F>::value>::type> {
#if have_optional
    inline static std::optional<T> to(F x)
    {
        if (std::is_unsigned<T>::value && std::is_signed<F>::value && x < 0) {
            return {};
        }

        using To = typename std::common_type<F, T>::type;
        if (std::is_signed<T>::value && std::is_signed<F>::value) {
            if (static_cast<To>(std::numeric_limits<T>::lowest()) > static_cast<To>(x)) {
                return {};
            }
        }
        if (static_cast<To>(std::numeric_limits<T>::max()) < static_cast<To>(x)) {
            return {};
        }
        return static_cast<T>(x);
    }
#endif

    inline static T convert(F x)
    {
        if (std::is_unsigned<T>::value && std::is_signed<F>::value && x < 0) {
            throw std::out_of_range("convert: value out of range " + std::to_string(x));
        }

        using To = typename std::common_type<F, T>::type;
        if (std::is_signed<T>::value && std::is_signed<F>::value) {
            if (static_cast<To>(std::numeric_limits<T>::lowest()) > static_cast<To>(x)) {
                throw std::out_of_range("convert: value out of range " + std::to_string(x));
            }
        }
        if (static_cast<To>(std::numeric_limits<T>::max()) < static_cast<To>(x)) {
            throw std::out_of_range("convert: value out of range " + std::to_string(x));
        }
        return static_cast<T>(x);
    }
};

/// Convert pair to another pair.
///
/// This is done by converting each element.
template <typename To1, typename To2, typename From1, typename From2>
struct Convert<std::pair<To1, To2>, std::pair<From1, From2>> {
#if have_optional
    inline static std::optional<std::pair<To1, To2>> to(const std::pair<From1, From2>& from)
    {
        auto x = Convert<To1, From1>::to(from.first);
        auto y = Convert<To2, From2>(from.second);
        if (x && y) return {*std::move(x), *std::move(y)};
        return {};
    }
#endif

    inline static std::pair<To1, To2> convert(const std::pair<From1, From2>& from)
    {
        return {Convert<To1, From1>::convert(from.first), Convert<To2, From2>::convert(from.second)};
    }
};

/// Convert tuple to another tuple.
///
/// This is done by converting each element.
template <typename... Ts, typename... Fs>
struct Convert<std::tuple<Ts...>, std::tuple<Fs...>> {
#if have_optional
    inline static std::optional<std::tuple<Ts...>> to(const std::tuple<Fs...>& tuple)
    {
        static constexpr auto size = std::tuple_size<std::tuple<Fs...>>::value;
        return to(tuple, std::make_index_sequence<size>{});
    }
#endif

    inline static std::tuple<Ts...> convert(const std::tuple<Fs...>& tuple)
    {
        static constexpr auto size = std::tuple_size<std::tuple<Fs...>>::value;
        return convert(tuple, std::make_index_sequence<size>{});
    }

private:
#if have_optional
    template <size_t... I>
    static std::optional<std::tuple<Ts...>> to(const std::tuple<Fs...>& args, std::index_sequence<I...>)
    {
        try {
            return std::make_tuple(
                Convert<Ts, typename std::remove_cv<typename std::remove_reference<Fs>::type>::type>::convert(
                    std::get<I>(args))...);
        } catch (...) {
            return {};
        }
    }
#endif

    template <size_t... I>
    static std::tuple<Ts...> convert(const std::tuple<Fs...>& args, std::index_sequence<I...>)
    {
        return std::make_tuple(
            Convert<Ts, typename std::remove_cv<typename std::remove_reference<Fs>::type>::type>::convert(
                std::get<I>(args))...);
    }
};

/// Convert array to another array.
///
/// This is done by converting each element.
template <typename T, typename F, std::size_t N>
struct Convert<std::array<T, N>, std::array<F, N>> {
#if have_optional
    inline static std::optional<std::array<T, N>> to(const std::array<F, N>& args)
    {
        std::array<T, N> result = {};
        for (std::size_t i = 0; i < N; i++) {
            auto x = Convert<T, F>::to(args[i]);
            if (x) {
                result[i] = *std::move(x);
            } else
                return {};
        }
        return result;
    }
#endif

    inline static std::array<T, N> convert(const std::array<F, N>& args)
    {
        std::array<T, N> result = {};
        std::transform(args.begin(), args.end(), result.begin(), Convert<T, F>::convert);
        return result;
    }
};

/// Convert vector to another vector.
///
/// This is done by converting each element.
template <typename T, typename F>
struct Convert<std::vector<T>, std::vector<F>> {
#if have_optional
    inline static std::optional<std::vector<T>> to(const std::vector<F>& args)
    {
        std::vector<T> result;
        result.reserve(args.size());
        for (auto& x : args) {
            auto y = Convert<T, F>::to(x);
            if (x) {
                result.push_back(*std::move(x));
            } else {
                return {};
            }
        }
        return result;
    }
#endif

    inline static std::vector<T> convert(const std::vector<F>& args)
    {
        std::vector<T> result;
        result.reserve(args.size());
        for (auto& x : args) {
            result.push_back(Convert<T, F>::convert(x));
        }
        return result;
    }
};

#if have_optional
/// Generic conversion function.
///
/// This function returns an `std::optional`. The returned value is valid if and
/// only if the conversion was successful.
template <typename T, typename F>
auto to(F&& x) -> std::optional<T>
{
    using From = typename std::remove_cv<typename std::remove_reference<F>::type>::type;
    return Convert<T, From>::to(std::forward<F>(x));
}
#endif

/// Generic conversion function.
template <typename T, typename F>
inline T convert(F&& x)
{
    using From = typename std::remove_cv<typename std::remove_reference<F>::type>::type;
    return Convert<T, From>::convert(std::forward<F>(x));  // NOLINT
}

#if have_optional
/// Generic conversion function for ranges.
///
/// This function converts a sequence given by a pair of iterators to a
/// resulting container. The result container must provide a `emplace_back`
/// method.
///
/// If at least one element could not be converted, the function returns an
/// invalid `std::optional`.
template <typename T, typename It>
inline std::optional<T> to(It beg, It end)
{
    T result;
    for (; beg != end; ++beg) {
        auto x = to<typename T::value_type>(std::move(*beg));
        if (x)
            result.emplace_back(std::move(*(x)));
        else
            return {};
    }
    return result;
}
#endif

/// Generic conversion function for ranges.
///
/// This function converts a sequence given by a pair of iterators to a
/// resulting container. The result container must provide a `emplace_back` method.
///
/// The function raises an error if at least one element could not be converted.
template <typename T, typename It>
inline T convert(It beg, It end)
{
    T result;
    for (; beg != end; ++beg) {
        result.emplace_back(convert<typename T::value_type>(std::move(*beg)));
    }
    return result;
}
}  // namespace util
}  // namespace fifr

#endif
