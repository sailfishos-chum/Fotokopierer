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

#ifndef __FIFR_UTIL_GZIPSTREAM_HXX__
#define __FIFR_UTIL_GZIPSTREAM_HXX__

#include "ZipStreamBase.hxx"

#include <memory>
#include <streambuf>
#include <string>

namespace fifr {
namespace util {
/// Stream buffer for gzipped files.
///
/// This class is based on @e gzstream.
class GZipStreamBuf : public std::streambuf
{
public:
    /// Constructor.
    GZipStreamBuf();

    GZipStreamBuf(GZipStreamBuf&&) = delete;
    GZipStreamBuf(const GZipStreamBuf&) = delete;
    GZipStreamBuf& operator=(GZipStreamBuf&&) = delete;
    GZipStreamBuf& operator=(const GZipStreamBuf&) = delete;

    /// Destructor.
    ///
    /// Closes the stream.
    ~GZipStreamBuf() override;

    /// Returns true if the associated file is opened.
    [[nodiscard]] bool is_open() const;

    ///
    /// Opens a file.
    ///
    /// A gzipped file may neither be opened read-write (at the
    /// same time) nor in append-mode.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    ///
    /// \return \b this on success, `nullptr` otherwise.
    ///
    GZipStreamBuf* open(const std::string& name, std::ios_base::openmode op_mode);

    /// Closes the associated file.
    GZipStreamBuf* close();

    int overflow(int c = EOF) override;

    int underflow() override;

    int sync() override;

private:
    /// Flushes the current write buffer to the file.
    int flush_buffer();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

using InputGZipStream = InputZipStreamBase<GZipStreamBuf>;

using OutputGZipStream = OutputZipStreamBase<GZipStreamBuf>;

using igzstream = InputGZipStream;

using ogzstream = OutputGZipStream;
}  // namespace util
}  // namespace fifr

#endif
