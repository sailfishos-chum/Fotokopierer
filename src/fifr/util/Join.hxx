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

#ifndef __FIFR_UTIL_JOIN_HXX__
#define __FIFR_UTIL_JOIN_HXX__

#include "Convert.hxx"

#include <array>
#include <sstream>
#include <string>

namespace fifr {
namespace util {
/**
 * Join operator.
 *
 * This object can be converted to a string, a C-string or written to
 * an output stream.
 */
template <class C>
class Join
{
public:
    Join(const C& container, const std::string& sep) : container_(container), sep_(sep) {}

    Join(Join&&) = delete;

    Join(const Join&) = delete;

    Join& operator=(Join&&) = delete;

    Join& operator=(const Join&) = delete;

    ~Join() = default;

    /// Return the joined string.
    [[nodiscard]] std::string str() const
    {
        std::ostringstream out;
        out << *this;
        return out.str();
    }

    operator std::string() const { return str(); }

    template <typename CC>
    friend std::ostream& operator<<(std::ostream& out, const Join<CC>& join);

private:
    const C& container_;
    const std::string& sep_;
};

/// Write a joined vector to an output stream.
template <class C>
std::ostream& operator<<(std::ostream& out, const Join<C>& join)
{
    auto it = join.container_.begin();
    auto itend = join.container_.end();
    if (it != itend) {
        out << *it;
        ++it;
        for (; it != itend; ++it) {
            out << join.sep_ << *it;
        }
    }
    return out;
}

/**
 * Join a container to a string.
 */
template <typename C>
Join<C> join(const C& container, std::string sep = "")
{
    return {container, std::move(sep)};
}

/// Add string and joined string.
template <typename C>
std::string operator+(const std::string& str, const Join<C>& join)
{
    return str + join.str();
}

/// Add joined string and string.
template <typename C>
std::string operator+(const Join<C>& join, const std::string& str)
{
    return join.str() + str;
}

/// Add string and joined string.
template <typename C>
std::string operator+(const char* str, const Join<C>& join)
{
    return str + join.str();
}

/// Add joined string and string.
template <typename C>
std::string operator+(const Join<C>& join, const char* str)
{
    return join.str() + str;
}

/**
 * Join a fixed number of objects.
 *
 * All objects are converted to `std::string` using
 * `fifr::util::convert` before being joined.
 *
 * @warning In contrast to other `join` functions, the separator is
 *          the *first* argument.
 */
template <typename... Args>
std::string join(const std::string& separator, Args&&... args)
{
    return join(std::array<std::string, sizeof...(Args)>{{convert<std::string>(std::forward<Args>(args))...}}, separator);
}
}  // namespace util
}  // namespace fifr

#endif
