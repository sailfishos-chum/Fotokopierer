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

#include "String.hxx"

namespace fifr::util {
bool starts_with(const std::string& str, const std::string& start)
{
    return (str.size() >= start.size() && str.compare(0, start.size(), start) == 0);
}

bool ends_with(const std::string& str, const std::string& end)
{
    return (str.size() >= end.size() && str.compare(str.size() - end.size(), end.size(), end) == 0);
}

std::string escape_shell_argument(const std::string& arg)
{
    std::string result;
    result.reserve(arg.size() + 2);
    std::string::size_type beg = 0;
    std::string::size_type pos;

    result.append("'");

    while ((pos = arg.find('\'', beg)) != std::string::npos) {
        result.append(arg.begin() + static_cast<std::string::difference_type>(beg),
                      arg.begin() + static_cast<std::string::difference_type>(pos));
        result.append("'\\''");
        beg = pos + 1;
    }

    result.append(arg.begin() + static_cast<std::string::difference_type>(beg), arg.end());
    result.append("'");

    return result;
}
}  // namespace fifr::util
