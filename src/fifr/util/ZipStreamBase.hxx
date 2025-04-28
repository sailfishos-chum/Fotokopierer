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

#ifndef __FIFR_UTIL_ZIPSTREAMBASE_HXX__
#define __FIFR_UTIL_ZIPSTREAMBASE_HXX__

#include <istream>
#include <ostream>

namespace fifr {
namespace util {
template <typename S>
class InputZipStreamBase : public std::istream
{
public:
    InputZipStreamBase() : std::istream(&buf_) {}

    /// Creates and opens a stream to a given file.
    ///
    /// The file is opened in read-mode by default.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    explicit InputZipStreamBase(const std::string& name, std::ios::openmode op_mode = std::ios::in)
        : InputZipStreamBase()
    {
        open(name, op_mode);
    }

    /// Opens a file.
    ///
    /// The file is opened in read-mode by default.
    ///
    /// If the stream is already open,  the file is closed and the
    /// bad-bit is set.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    void open(const std::string& name, std::ios::openmode op_mode = std::ios::in)
    {
        if (!buf_.open(name, op_mode)) {
            clear(rdstate() | std::ios::badbit);
        }
    }

    /// Close the underlying file.
    void close()
    {
        if (buf_.is_open() && buf_.close() == nullptr) {
            clear(rdstate() | std::ios::badbit);
        }
    }

private:
    S buf_;
};

template <typename S>
class OutputZipStreamBase : public std::ostream
{
public:
    OutputZipStreamBase() : std::ostream(&buf_) {}

    /// Creates and opens a stream to a given file.
    ///
    /// The file is opened in write-mode by default.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    explicit OutputZipStreamBase(const std::string& name, std::ios::openmode op_mode = std::ios::out)
        : OutputZipStreamBase()
    {
        open(name, op_mode);
    }

    /// Opens a file.
    ///
    /// The file is opened in read-mode by default.
    ///
    /// If the stream is already open,  the file is closed and the
    /// bad-bit is set.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    void open(const std::string& name, std::ios::openmode op_mode = std::ios::out)
    {
        if (!buf_.open(name, op_mode)) {
            clear(rdstate() | std::ios::badbit);
        }
    }

    /// Close the underlying file.
    void close()
    {
        if (buf_.is_open() && buf_.close() == nullptr) {
            clear(rdstate() | std::ios::badbit);
        }
    }

private:
    S buf_;
};
}  // namespace util
}  // namespace fifr

#endif
