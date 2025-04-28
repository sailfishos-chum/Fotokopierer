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

#ifndef __FIFR_UTIL_PRETTYSTRUCT_HXX__
#define __FIFR_UTIL_PRETTYSTRUCT_HXX__

#include "IndentStream.hxx"
#include "Pretty.hxx"

#include <cassert>

namespace fifr {
namespace util {
/// Marker that an argument should be written in multiline style.
template <typename T>
struct MultiLine {
    const T& value;
};

/// Mark an argument to be written in multiline style.
template <typename T>
MultiLine<T> multiline(const T& x)
{
    return MultiLine<T>{x};
}

template <typename T>
void pretty_print(IndentStream& out, const MultiLine<T>& arg)
{
    pretty_print(out, arg.value, true);
}

namespace intern {
void pretty_read_struct_name(std::istream& in, detail::string_view_ref name);
void pretty_read_struct_field_name(std::istream& in, std::string& param);

template <std::size_t N>
void pretty_read_struct_check_missing(std::array<bool, N>&)
{
}

template <std::size_t I = 0, std::size_t N, typename Arg, typename... Args>
void pretty_read_struct_check_missing(std::array<bool, N>& seen, detail::string_view_ref param, Arg&, Args&... args)
{
    if (!seen[I]) {
        throw ReadError("Missing field: " + std::string(param));
    }
    pretty_read_struct_check_missing<I + 1>(seen, args...);  // NOLINT
}

template <std::size_t N>
void pretty_read_struct_field(std::istream&, detail::string_view_ref name, std::array<bool, N>&)
{
    throw ReadError("Unknown struct field: " + std::string(name));
}

template <std::size_t I = 0, std::size_t N, typename Arg, typename... Args>
void pretty_read_struct_field(std::istream& in,
                              detail::string_view_ref name,
                              std::array<bool, N>& seen,
                              detail::string_view_ref field,
                              Arg& arg,
                              Args&... args)
{
    if (name == field) {
        if (seen[I]) {
            throw ReadError("Duplicate field " + std::string(name));
        }
        seen[I] = true;
        in >> std::ws;
        try {
            pretty_read(in, arg);
        } catch (std::istream::failure& fail) {
            throw ReadError("Cannot extract value for field " + std::string(name) + "(" + fail.what() + ")");
        }
    } else {
        pretty_read_struct_field<I + 1>(in, name, seen, args...);  // NOLINT
    }
}

}  // namespace intern

/// Read a struct from a simple formatted string representation.
///
/// The string must be of the form
///
///     MyStruct[a:1, b:2, c:3]
///
/// where `MyStruct` is an arbitrary identifier (name of the struct) and
/// `a`, `b` and `c` are the names of the fields. The struct is deserialized using
///
///     int a;
///     unsigned b;
///     double c;
///
///     pretty_read_struct(in, "MyStruct", "a", a, "b", b, "c", c);
///
/// If an error occurs, the exception `ReadError` is thrown.
template <typename... Args>
void pretty_read_struct(std::istream& in, detail::string_view_ref name, Args&... args)
{
    assert(!name.empty());

    auto exception_flags = in.exceptions();
    in.exceptions(std::istream::failbit);

    static constexpr auto size = std::tuple_size<std::tuple<Args...>>::value;
    static_assert(size % 2 == 0, "Even number of field/value parameters is required");
    std::array<bool, size / 2> seen = {};

    try {
        intern::pretty_read_struct_name(in, name);

        in >> std::ws;
        if (in.get() != '[') {
            throw ReadError("Unexpected character after struct name " + std::string(name) + ", expected '['");
        }

        std::string param;
        in >> std::ws;
        while (true) {
            intern::pretty_read_struct_field_name(in, param);
            intern::pretty_read_struct_field(in, param, seen, args...);  // NOLINT

            in >> std::ws;

            auto c = in.get();
            if (c == std::char_traits<char>::eof()) {
                throw ReadError("Unexpected eof after field, expected ',' or ']'");
            }
            if (c != ']' && c != ',') {
                throw ReadError("Unexpected character after field, expected ',' or ']', got: '" +
                                std::string(1, std::char_traits<char>::to_char_type(c)) + "'");
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

        intern::pretty_read_struct_check_missing(seen, args...);  // NOLINT
    } catch (std::istream::failure& fail) {
        in.exceptions(exception_flags);
        throw ReadError("Error reading the struct: " + std::string(fail.what()));
    } catch (...) {
        in.exceptions(exception_flags);
        throw;
    }
}

namespace intern {
template <bool first, bool multiline>
void pretty_print_struct_field(IndentStream&)
{
}

template <bool first = true, bool multiline = false, typename Arg, typename... Args>
void pretty_print_struct_field(IndentStream& out, detail::string_view_ref field, const Arg& arg, const Args&... args)
{
    if (!multiline) {
        if (!first) {
            out << ", ";
        }
    } else {
        out << '\n';
    }
    out << field << ": ";
    pretty_print(out, arg);  // NOLINT
    if (multiline) {
        out << ',';
    }

    pretty_print_struct_field<false, multiline>(out, args...);  // NOLINT
}
}  // namespace intern

/// Write a struct to a simple formatted string representation.
///
/// The string will be of the form, everything on a single line
///
///     MyStruct[a:1, b:2, c:3]
///
/// where `MyStruct` is an arbitrary identifier (name of the struct) and
/// `a`, `b` and `c` are the names of the fields. The struct is serialized using
///
///     pretty_print_struct(in, "MyStruct", "a", 1, "b", 2, "c", 3);
template <typename... Args>
void pretty_print_struct(std::ostream& out, detail::string_view_ref name, const Args&... args)
{
    IndentStream wout(out);
    wout << name << '[';
    wout.increase();
    intern::pretty_print_struct_field<true, false>(wout, args...);  // NOLINT
    wout.decrease();
    wout << ']';
}

/// Pretty print a struct in multiline layout
template <typename... Args>
void pretty_print_struct_multiline(std::ostream& out, detail::string_view_ref name, const Args&... args)
{
    IndentStream wout(out);
    wout << name << '[';
    wout.increase();
    intern::pretty_print_struct_field<true, true>(wout, args...);  // NOLINT
    wout << '\n';
    wout.decrease();
    wout << ']';
}
}  // namespace util
}  // namespace fifr

#endif
