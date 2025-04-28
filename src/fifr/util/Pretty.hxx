/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_PRETTY_HXX__
#define __FIFR_UTIL_PRETTY_HXX__

#include "IndentStream.hxx"

#include <cassert>
#include <istream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace fifr {
namespace util {
namespace detail {
#ifdef __cpp_lib_string_view
using string_view = std::string_view;
using string_view_ref = std::string_view;
#else
using string_view = std::string;
using string_view_ref = const std::string&;
#endif
}  // namespace detail

/// Error when reading data from an input stream.
class ReadError : public std::runtime_error
{
public:
    explicit ReadError(const std::string& msg) : std::runtime_error(msg) {}
};

void pretty_print(IndentStream& out, detail::string_view_ref arg);

inline void pretty_print(IndentStream& out, const char* arg)
{
    pretty_print(out, detail::string_view{arg});
}

#ifdef __cpp_lib_string_view
inline void pretty_print(IndentStream& out, const std::string& arg)
{
    pretty_print(out, std::string_view{arg});
}
#endif

void pretty_print(IndentStream& out, const bool& arg);

template <typename Arg>
void pretty_print(IndentStream& out, const Arg& arg)
{
#ifdef __cpp_if_constexpr
    if constexpr (std::is_integral<Arg>::value) {  // NOLINT
        if (arg == std::numeric_limits<Arg>::max()) {
            out << "max";
        } else if (std::is_signed<Arg>::value && arg == std::numeric_limits<Arg>::lowest()) {
            out << "min";
        } else {
            out << arg;
        }
        return;
    }
#endif
    out << arg;
}

template <typename Arg>
void pretty_print(IndentStream& out, const std::unique_ptr<Arg>& arg)
{
    pretty_print(out, *arg);
}

template <typename... Arg>
void pretty_print_tuple(IndentStream&, const std::tuple<Arg...>&, bool, std::index_sequence<>)
{
}

template <typename... Arg, std::size_t I, std::size_t... Is>
void pretty_print_tuple(IndentStream& out, const std::tuple<Arg...>& arg, bool multiline, std::index_sequence<I, Is...>)
{
    if (!multiline) {
        if (I > 0) {
            out << ", ";
        }
        pretty_print(out, std::get<I>(arg));
    } else {
        out.increase();
        out << '\n';
        pretty_print(out, std::get<I>(arg));
        out.decrease();
        out << ',';
    }
    pretty_print_tuple(out, arg, multiline, std::index_sequence<Is...>{});
}

template <typename... Arg>
void pretty_print(IndentStream& out, const std::tuple<Arg...>& arg, bool multiline = false)
{
    out << '(';
    pretty_print_tuple(out, arg, multiline, std::index_sequence_for<Arg...>{});
    if (!multiline) {
        out << ')';
    } else {
        out << "\n)";
    }
}

template <typename Arg1, typename Arg2>
void pretty_print(IndentStream& out, const std::pair<Arg1, Arg2>& arg, bool multiline = false)
{
    pretty_print(out, std::tuple<Arg1, Arg2>(arg), multiline);
}

template <typename Arg>
void pretty_print(IndentStream& out, const std::vector<Arg>& arg, bool multiline = false);

template <typename Arg, std::size_t N>
void pretty_print(IndentStream& out, const std::array<Arg, N>& arg, bool multiline = false)
{
    pretty_print(out, std::vector<Arg>(arg.begin(), arg.end()), multiline);
}

template <typename Arg>
void pretty_print(IndentStream& out, const std::vector<Arg>& arg, bool multiline)
{
    if (!multiline) {
        out << '[';
        for (std::size_t i = 0; i < arg.size(); i++) {
            if (i > 0) {
                {
                    out << ", ";
                }
            }
            pretty_print(out, arg[i]);
        }
        out << ']';
    } else {
        out << '[';
        for (auto it = arg.begin(), it_end = arg.end(); it != it_end; ++it) {
            out.increase();
            out << '\n';
            pretty_print(out, *it);
            out.decrease();
            out << ',';
        }
        out << "\n]";
    }
}

template <typename Map>
void pretty_print_map(IndentStream& out, const Map& arg, bool multiline)
{
    if (!multiline) {
        out << '{';
        bool first = true;
        for (auto& it : arg) {
            if (!first) {
                {
                    out << ", ";
                }
            } else {
                {
                    first = false;
                }
            }
            pretty_print(out, it.first);
            out << ": ";
            pretty_print(out, it.second);
        }
        out << '}';
    } else {
        out << '{';
        for (auto& it : arg) {
            out.increase();
            out << '\n';
            pretty_print(out, it.first);
            out << ": ";
            pretty_print(out, it.second);
            out << ',';
            out.decrease();
        }
        out << "\n}";
    }
}

template <typename K, typename V>
void pretty_print(IndentStream& out, const std::map<K, V>& arg, bool multiline = false)
{
    pretty_print_map(out, arg, multiline);
}

template <typename K, typename V>
void pretty_print(IndentStream& out, const std::unordered_map<K, V>& arg, bool multiline = false)
{
    pretty_print_map(out, arg, multiline);
}

template <typename Arg>
void pretty_print(IndentStream& out, const Arg& arg, bool)
{
    pretty_print(out, arg);
}

void pretty_read(std::istream& in, bool& arg);

template <typename Arg>
void pretty_read(std::istream& in, Arg& arg)
{
    in >> std::ws;
#ifdef __cpp_if_constexpr
    if constexpr (std::is_integral<Arg>::value) {  // NOLINT
        if (in.get() == 'm') {
            auto ch = in.get();
            if (ch == 'a' && in.get() == 'x' && !std::isalnum(in.get())) {
                arg = std::numeric_limits<Arg>::max();
            } else if (ch == 'i' && in.get() == 'n' && !std::isalnum(in.get())) {
                arg = std::numeric_limits<Arg>::lowest();
            } else {
                throw ReadError("Expected 'max', 'min', '-' or a digit for integral value");
            }
            in.unget();
        } else {
            in.unget();
            in >> arg;
        }
        return;
    }

    if constexpr (std::is_floating_point<Arg>::value) {  // NOLINT
        auto ch = in.get();
        if (ch == 'i') {
            if (in.get() == 'n' && in.get() == 'f' && !std::isalnum(in.get())) {
                arg = std::numeric_limits<Arg>::infinity();
            } else {
                throw ReadError("Expected 'inf', '-inf' or a floating point value");
            }
            in.unget();
        } else if (ch == '-') {
            pretty_read(in, arg);
            arg = -arg;
        } else {
            in.unget();
            in >> arg;
        }
        return;
    }
#endif
    in >> arg;
}

template <typename Arg>
void pretty_read(std::istream& in, std::unique_ptr<Arg>& arg)
{
    arg.reset(new Arg());
    pretty_read(in, *arg);
}

void pretty_read(std::istream& in, std::string& arg);

template <typename... Arg>
void pretty_read_tuple(std::istream& in, std::tuple<Arg...>&, std::index_sequence<>)
{
    in >> std::ws;
    if (in.get() == ',') {
        in >> std::ws;
    } else {
        in.unget();
    }
    if (in.get() != ')') {
        throw ReadError("Expected ')' character at the end of a tuple");
    }
}

template <typename... Arg, std::size_t I, std::size_t... Is>
void pretty_read_tuple(std::istream& in, std::tuple<Arg...>& arg, std::index_sequence<I, Is...>)
{
    if (I != 0) {
        in >> std::ws;
        if (in.get() != ',') {
            throw ReadError("Expected ',' character between tuple elements");
        }
    }
    in >> std::ws;
    pretty_read(in, std::get<I>(arg));
    pretty_read_tuple(in, arg, std::index_sequence<Is...>{});
}

template <typename... Arg>
void pretty_read(std::istream& in, std::tuple<Arg...>& arg)
{
    in >> std::ws;
    if (in.get() != '(') {
        throw ReadError("Expected '(' character for tuple value");
    }

    pretty_read_tuple(in, arg, std::index_sequence_for<Arg...>{});
}

template <typename Arg1, typename Arg2>
void pretty_read(std::istream& in, std::pair<Arg1, Arg2>& arg)
{
    std::tuple<Arg1, Arg2> x;
    pretty_read(in, x);
    arg.first = std::move(std::get<0>(x));
    arg.second = std::move(std::get<1>(x));
}

template <typename Arg>
void pretty_read(std::istream& in, std::vector<Arg>& arg);

template <typename Arg, std::size_t N>
void pretty_read(std::istream& in, std::array<Arg, N>& arg)
{
    std::vector<Arg> x;
    pretty_read(in, x);
    if (x.size() != N) {
        throw ReadError("Expected array of exactly " + std::to_string(N) +
                        " elements (got: " + std::to_string(x.size()) + ")");
    }
    std::move(x.begin(), x.end(), arg.begin());
}

template <typename Arg>
void pretty_read(std::istream& in, std::vector<Arg>& arg)
{
    arg.clear();
    if (in.get() != '[') {
        throw ReadError("Expected '[' character for array value");
    }

    in >> std::ws;
    if (in.get() == ']') {
        return;  // empty array
    }
    in.unget();

    in >> std::ws;
    for (;;) {
        Arg a;
        pretty_read(in, a);
        arg.push_back(std::move(a));

        in >> std::ws;
        auto c = in.get();
        if (c != ',' && c != ']') {
            throw ReadError("Expected ',' or ']' character after array element");
        }
        if (c == ',') {
            in >> std::ws;
            c = in.get();
        }
        if (c == ']') {
            break;
        }
        in.unget();
    }
}

template <typename Map>
void pretty_read_map(std::istream& in, Map& arg)
{
    arg.clear();
    if (in.get() != '{') {
        throw ReadError("Expected '{' character for map value");
    }

    in >> std::ws;
    if (in.get() == '}') {
        return;  // empty map
    }
    in.unget();

    in >> std::ws;
    for (;;) {
        typename Map::key_type key;
        pretty_read(in, key);

        in >> std::ws;
        if (in.get() != ':') {
            throw ReadError("Expected ':' character after key in map");
        }
        in >> std::ws;

        typename Map::mapped_type value;
        pretty_read(in, value);

        arg.insert({std::move(key), std::move(value)});

        in >> std::ws;
        auto c = in.get();
        if (c != ',' && c != '}') {
            throw ReadError("Expected ',' or '}' character after value in map");
        }
        if (c == ',') {
            in >> std::ws;
            c = in.get();
        }
        if (c == '}') {
            break;
        }
        in.unget();
    }
}

template <typename K, typename V>
void pretty_read(std::istream& in, std::map<K, V>& arg)
{
    pretty_read_map(in, arg);
}

template <typename K, typename V>
void pretty_read(std::istream& in, std::unordered_map<K, V>& arg)
{
    pretty_read_map(in, arg);
}

/// Pretty printing tag.
template <typename T>
struct Pretty {
    T value;
    bool multiline;
};

/// This function uses SimpleStructIO functions to pretty print a plain value.
template <typename T>
Pretty<T> pretty(T&& arg, bool multiline = false)
{
    return {std::forward<T>(arg), multiline};
}

template <typename T>
std::ostream& operator<<(std::ostream& out, Pretty<T>&& pretty)
{
    IndentStream iout(out);
    pretty_print(iout, pretty.value, pretty.multiline);
    return out;
}

template <typename T>
std::istream& operator>>(std::istream& in, Pretty<T>&& pretty)
{
    pretty_read(in, pretty.value);
    return in;
}

}  // namespace util
}  // namespace fifr

#endif
