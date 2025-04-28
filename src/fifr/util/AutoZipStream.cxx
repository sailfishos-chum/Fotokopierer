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

#include "AutoZipStream.hxx"

#include "Join.hxx"
#include "String.hxx"
#include "ToolStream.hxx"

#include <ext/stdio_filebuf.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

#ifdef HAVE_GZIP
#include "GZipStream.hxx"
#endif

#ifdef HAVE_BZIP2
#include "BZip2Stream.hxx"
#endif

using namespace std::literals;

namespace fifr::util {
class AutoZipStreamData
{
public:
    open_streambuf open;
    ZipMode zipmode;
    std::unique_ptr<std::streambuf> buf;
};

std::unique_ptr<std::streambuf> open_by_extension(const std::string& name, std::ios::openmode opmode, ZipMode zipmode)
{
    if (ends_with(name, ".gz")) {
#ifdef HAVE_GZIP
        if (zipmode == ZipMode::Auto || zipmode == ZipMode::Internal) {
            std::unique_ptr<GZipStreamBuf> f{new GZipStreamBuf()};
            if (f->open(name, opmode) == nullptr) {
                return nullptr;
            }
            return f;
        }
#endif
        if (zipmode == ZipMode::Auto || zipmode == ZipMode::External) {
            if ((opmode & std::ios::in) != 0) {
                return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/gzip", {"-c", "-d", name}, opmode));
            }
            return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/gzip", {"-"}, name, opmode));
        }
        std::cerr << "Internal compression for bzip2 not compiled in";
        std::abort();

    } else if (ends_with(name, ".bz2")) {
#ifdef HAVE_BZIP2
        if (zipmode == ZipMode::Auto || zipmode == ZipMode::Internal) {
            std::unique_ptr<BZip2StreamBuf> f{new BZip2StreamBuf()};
            if (f->open(name, opmode) == nullptr) {
                return nullptr;
            }
            return f;
        }
#endif
        if (zipmode == ZipMode::Auto || zipmode == ZipMode::External) {
            if ((opmode & std::ios::in) != 0) {
                return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/bzip2", {"-c", "-d", name}, opmode));
            }
            return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/bzip2", {"-"}, name, opmode));
        }
        std::cerr << "Internal compression for bzip2 not compiled in";
        std::abort();

    } else if (ends_with(name, ".xz")) {
        if (zipmode == ZipMode::Auto || zipmode == ZipMode::External) {
            if ((opmode & std::ios::in) != 0) {
                return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/xz", {"-c", "-d", name}, opmode));
            }
            return std::unique_ptr<std::streambuf>(new ToolStreambuf("/usr/bin/xz", {"-"}, name, opmode));
        }
        std::cerr << "Internal compression for xz not compiled in";
        std::abort();

    } else {
        std::unique_ptr<std::filebuf> f{new std::filebuf()};
        f->open(name, opmode);
        return f;
    }
}

InputAutoZipStream::InputAutoZipStream(ZipMode zipmode, const open_streambuf& open)
    : d(new AutoZipStreamData{open, zipmode, {}})
{
}

InputAutoZipStream::InputAutoZipStream(const std::string& name,
                                       std::ios::openmode op_mode,
                                       ZipMode zipmode,
                                       const open_streambuf& open)
    : InputAutoZipStream(zipmode, open)
{
    this->open(name, op_mode);
}

InputAutoZipStream::InputAutoZipStream(InputAutoZipStream&& s) noexcept : d(std::move(s.d)) {}

InputAutoZipStream::~InputAutoZipStream() = default;

InputAutoZipStream& InputAutoZipStream::operator=(InputAutoZipStream&& s) noexcept
{
    d = std::move(s.d);
    return *this;
}

void InputAutoZipStream::open(const std::string& name, std::ios_base::openmode op_mode)
{
    rdbuf(nullptr);
    clear();
    d->buf = d->open(name, op_mode, d->zipmode);
    rdbuf(d->buf.get());
    if (d->buf == nullptr) {
        clear(rdstate() | std::ios::badbit);
    }
}

OutputAutoZipStream::OutputAutoZipStream(ZipMode zipmode, const open_streambuf& open)
    : d(new AutoZipStreamData{open, zipmode, {}})
{
}

OutputAutoZipStream::OutputAutoZipStream(const std::string& name,
                                         std::ios::openmode op_mode,
                                         ZipMode zipmode,
                                         const open_streambuf& open)
    : OutputAutoZipStream(zipmode, open)
{
    this->open(name, op_mode);
}

OutputAutoZipStream::OutputAutoZipStream(OutputAutoZipStream&& s) noexcept : d(std::move(s.d)) {}

OutputAutoZipStream::~OutputAutoZipStream() = default;

OutputAutoZipStream& OutputAutoZipStream::operator=(OutputAutoZipStream&& s) noexcept
{
    d = std::move(s.d);
    return *this;
}

void OutputAutoZipStream::open(const std::string& name, std::ios_base::openmode op_mode)
{
    rdbuf(nullptr);
    clear();
    d->buf = d->open(name, op_mode, d->zipmode);
    rdbuf(d->buf.get());
    if (d->buf == nullptr) {
        clear(rdstate() | std::ios::badbit);
    }
}
}  // namespace fifr::util
