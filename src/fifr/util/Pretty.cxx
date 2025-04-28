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

#include "Pretty.hxx"

#include <string_view>

namespace fifr::util {
void pretty_read(std::istream& in, std::string& arg)
{
    if (in.get() != '"') {
        throw ReadError("Expected \" for string value");
    }
    arg.clear();
    for (auto c = in.get(); c != '"'; c = in.get()) {
        if (c == '\\') {
            c = in.get();
            switch (c) {
                case 'r': arg.push_back('\r'); break;
                case 'n': arg.push_back('\n'); break;
                case 't': arg.push_back('\t'); break;
                case '0': arg.push_back('\0'); break;
                default: arg.push_back(std::char_traits<char>::to_char_type(c)); break;
            }
        } else {
            arg.push_back(std::char_traits<char>::to_char_type(c));
        }
    }
}

void pretty_read(std::istream& in, bool& arg)
{
    in >> std::ws;

    auto ch = in.get();

    if (ch == 't') {
        if (in.get() == 'r' && in.get() == 'u' && in.get() == 'e' && std::isalnum(in.get()) == 0) {
            in.unget();
            arg = true;
        } else {
            throw ReadError("Expected 'true', 'false' or a number of boolean value");
        }
    } else if (ch == 'f') {
        if (in.get() == 'a' && in.get() == 'l' && in.get() == 's' && in.get() == 'e' && std::isalnum(in.get()) == 0) {
            in.unget();
            arg = false;
        } else {
            throw ReadError("Expected 'true', 'false' or a number of boolean value");
        }
    } else {
        in.unget();
        in >> arg;
    }
}

void pretty_print(IndentStream& out, detail::string_view_ref arg)
{
    static const detail::string_view ctrls("\\\"\n\r\t\0", 6);

    out << '"';
    for (detail::string_view::size_type pos = 0, end;; pos = end + 1) {
        end = arg.find_first_of(ctrls, pos);
        if (end == detail::string_view::npos) {
            out << arg.substr(pos);
            break;
        }
        out << arg.substr(pos, end - pos) << '\\';
        switch (arg[end]) {
            case '\n': out << 'n'; break;
            case '\r': out << 'r'; break;
            case '\t': out << 't'; break;
            case '\0': out << '0'; break;
            default: out << arg[end]; break;
        }
    }
    out << '"';
}

void pretty_print(IndentStream& out, const bool& arg)
{
    out << (arg ? "true" : "false");
}

}  // namespace fifr::util
