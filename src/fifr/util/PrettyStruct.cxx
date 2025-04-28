/*
 * Copyright (c) 2018-2020 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "PrettyStruct.hxx"

#include "Join.hxx"
#include "Range.hxx"

namespace fifr::util::intern {
void pretty_read_struct_name(std::istream& in, detail::string_view_ref name)
{
    in >> std::ws;
    for (auto pos : range(name.size())) {
        int c = in.get();
        if (c == std::char_traits<char>::eof()) {
            throw ReadError("Unexpected eof when reading struct name");
        }
        if (name[pos] != c) {
            throw ReadError("Invalid struct name, expected '" + std::string(name) + "' got: '" +
                            std::string(name.substr(0, pos)) + std::string(1, static_cast<char>(c)) + "'");
        }
    }
}

void pretty_read_struct_field_name(std::istream& in, std::string& param)
{
    param.clear();
    in >> std::ws;

    for (auto c = in.get(); (std::isalnum(c) != 0) || c == '_'; c = in.get()) {
        param.push_back(std::char_traits<char>::to_char_type(c));
    }

    in.unget();
    in >> std::ws;
    auto c = in.get();

    if (c == std::char_traits<char>::eof()) {
        throw ReadError("Unexpected eof after field name, expected ':'");
    }
    if (c != ':') {
        throw ReadError("Unexpected character after field name, expected ':', got: '" +
                        std::string(1, std::char_traits<char>::to_char_type(c)) + "'");
    }
}

}  // namespace fifr::util::intern
