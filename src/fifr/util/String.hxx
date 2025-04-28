/*
 * Copyright (c) 2016, 2017, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_STRING_HXX__
#define __FIFR_UTIL_STRING_HXX__

#include <string>

namespace fifr {
/// Utility functions for c++.
namespace util {
/// Returns true if a string starts with a certain substring.
bool starts_with(const std::string& str, const std::string& start);

/// Returns true if a string ends with a certain substring.
bool ends_with(const std::string& str, const std::string& end);

/**
 * Quote a string for being passed as a shell argument.
 *
 * Replace all single quotes by '\'' and enclose the whole string
 * within single quotes.
 */
std::string escape_shell_argument(const std::string& arg);

}  // namespace util
}  // namespace fifr

#endif
