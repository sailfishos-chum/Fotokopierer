/*
 * Copyright (c) 2017 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_WORDWRAPSTREAM_HXX__
#define __FIFR_UTIL_WORDWRAPSTREAM_HXX__

#include <memory>
#include <ostream>

namespace fifr {
namespace util {
/// Write to an output stream with word wrapping.
///
/// This is a simple wrapper around `std::ostream`. Data written to this
/// stream is buffered and written with word wrap.
///
/// Word wrap can be customized by the following parameters:
///
///  - width ... the maximal width of a line
///  - indent ... the indentation of of all lines
///  - indent_first ... if set the indentation of the first line.
struct WordWrapStream {
    template <typename T>
    friend WordWrapStream& operator<<(WordWrapStream& out, const T& value);

public:
    static const std::size_t DEFAULT_WIDTH = 80;

public:
    explicit WordWrapStream(std::ostream& out);

    WordWrapStream(WordWrapStream&&) = default;
    WordWrapStream(const WordWrapStream&) = delete;
    WordWrapStream& operator=(WordWrapStream&&) = default;
    WordWrapStream& operator=(const WordWrapStream&) = delete;

    ~WordWrapStream();

    /// Write current buffer to output stream.
    void flush();

    /// Set the width at which lines should be wrapped.
    void set_width(std::size_t width);

    /// Set the indentation of the lines.
    void set_indent(std::size_t indent);

    /// Set the indentation of the first line.
    void set_indent_first(std::size_t indent);

    /// Insert spaces until the given column.
    ///
    /// If the current position is beyond that column, nothing is written.
    void shift_to(std::size_t pos);

private:
    std::ostream& stream();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

/// Write `value` to a word wrapping stream.
template <typename T>
WordWrapStream& operator<<(WordWrapStream& out, const T& value)
{
    out.stream() << value;  // NOLINT
    return out;
}
}  // namespace util
}  // namespace fifr

#endif
