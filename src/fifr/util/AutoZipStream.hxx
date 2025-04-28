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

#ifndef __FIFR_UTIL_AUTOZIPSTREAM_HXX__
#define __FIFR_UTIL_AUTOZIPSTREAM_HXX__

#include <functional>
#include <istream>
#include <memory>
#include <ostream>

namespace fifr {
namespace util {
/// Selection of an internal or external compressor.
enum class ZipMode {
    /// Automatically select the compressor.
    ///
    /// If the correct compression library is available, choose that
    /// library. Otherwise choose an external compression tool
    Auto,
    /// Always use the linked compression library (if available).
    Internal,
    /// Always use the external compression tool.
    External,
};

/// A function to open a streambuf depending on filename, openmode and zipmode.
using open_streambuf = std::function<std::unique_ptr<std::streambuf>(const std::string&, std::ios::openmode, ZipMode)>;

/// Open a streambuf depending on the file extension.
///
/// This function detects the compression type of a file
/// automatically and chooses the appropriate compressor.
///
///  - `.gz`  zlib (gzip/zcat)
///  - `.bz2` bzip2 (bzip2/bzcat)
///  - `.xz` xz (xz/xzcat)
std::unique_ptr<std::streambuf> open_by_extension(const std::string& name, std::ios::openmode, ZipMode zipmode);

class AutoZipStreamData;

/// A file stream that automatically decompresses files.
///
/// This stream detects the compression type of a file
/// automatically and chooses the appropriate compressor.
///
/// The default implementation chooses the compressor based
/// on the file extension:
///
///  - `.gz`  zlib (gzip/zcat)
///  - `.bz2` bzip2 (bzip2/bzcat)
///  - `.xz` xz (xz/xzcat)
class InputAutoZipStream : public std::istream
{
public:
    explicit InputAutoZipStream(ZipMode zipmode = ZipMode::Auto, const open_streambuf& open = open_by_extension);

    explicit InputAutoZipStream(const std::string& name,
                                std::ios::openmode op_mode = std::ios::in,
                                ZipMode zipmode = ZipMode::Auto,
                                const open_streambuf& open = open_by_extension);

    InputAutoZipStream(InputAutoZipStream&& s) noexcept;

    InputAutoZipStream(const InputAutoZipStream&) = delete;

    ~InputAutoZipStream() override;

    InputAutoZipStream& operator=(InputAutoZipStream&& s) noexcept;

    InputAutoZipStream& operator=(const InputAutoZipStream&) = delete;

    void open(const std::string& name, std::ios_base::openmode op_mode = std::ios::in);

private:
    std::unique_ptr<AutoZipStreamData> d;
};

/// A file stream that automatically compresses files.
///
/// This stream detects the compression type of a file
/// automatically and chooses the appropriate compressor.
///
/// The default implementation chooses the compressor based
/// on the file extension:
///
///  - `.gz`  zlib (gzip/zcat)
///  - `.bz2` bzip2 (bzip2/bzcat)
///  - `.xz` xz (xz/xzcat)
class OutputAutoZipStream : public std::ostream
{
public:
    explicit OutputAutoZipStream(ZipMode zipmode = ZipMode::Auto, const open_streambuf& open = open_by_extension);

    explicit OutputAutoZipStream(const std::string& name,
                                 std::ios::openmode op_mode = std::ios::out,
                                 ZipMode zipmode = ZipMode::Auto,
                                 const open_streambuf& open = open_by_extension);

    OutputAutoZipStream(OutputAutoZipStream&& s) noexcept;

    OutputAutoZipStream(const OutputAutoZipStream& s) noexcept = delete;

    ~OutputAutoZipStream() override;

    OutputAutoZipStream& operator=(OutputAutoZipStream&& s) noexcept;

    OutputAutoZipStream& operator=(const OutputAutoZipStream& s) noexcept = delete;

    void open(const std::string& name, std::ios_base::openmode op_mode = std::ios::out);

private:
    std::unique_ptr<AutoZipStreamData> d;
};

using izipstream = InputAutoZipStream;
using ozipstream = OutputAutoZipStream;
}  // namespace util
}  // namespace fifr

#endif
