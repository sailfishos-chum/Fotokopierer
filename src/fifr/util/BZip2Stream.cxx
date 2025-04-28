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

#include "BZip2Stream.hxx"

#include <bzlib.h>

#include <array>
#include <cstdio>
#include <cstring>

namespace fifr::util {
static const unsigned BufferSize = 47 + 256;
static const int BlockSize100k = 9;

struct BZip2StreamBuf::Data {
    FILE* file = nullptr;
    BZFILE* bzfile = nullptr;
    std::array<char, BufferSize> buf = {};
    bool is_open = false;
    std::ios::openmode mode = {};
};

BZip2StreamBuf::BZip2StreamBuf() : d(new Data)
{
    setp(d->buf.data(), d->buf.data() + (BufferSize - 1));
    setg(d->buf.data() + 4, d->buf.data() + 4, d->buf.data() + 4);
}

BZip2StreamBuf::~BZip2StreamBuf()
{
    close();
}

bool BZip2StreamBuf::is_open() const
{
    return d->is_open;
}

BZip2StreamBuf* BZip2StreamBuf::open(const std::string& name, std::ios::openmode op_mode)
{
    if (is_open()) {
        close();
        return nullptr;
    }

    d->mode = op_mode;

    if (((d->mode & std::ios::ate) != 0) || ((d->mode & std::ios::app) != 0) ||
        (((d->mode & std::ios::in) != 0) && ((d->mode & std::ios::out) != 0))) {
        return nullptr;
    }

    std::array<char, 3> fmode = {};
    std::size_t pos = 0;

    if ((d->mode & std::ios::in) != 0) {
        fmode.at(pos++) = 'r';
    } else if ((d->mode & std::ios::out) != 0) {
        fmode.at(pos++) = 'w';
    }
    fmode.at(pos++) = 'b';
    fmode.at(pos++) = '0';

    d->file = fopen(name.c_str(), &fmode[0]);
    if (d->file == nullptr) {
        return nullptr;
    }

    int bzerror;
    if ((d->mode & std::ios::in) != 0) {
        d->bzfile = BZ2_bzReadOpen(&bzerror, d->file, 0, 0, nullptr, 0);
    } else if ((d->mode & std::ios::out) != 0) {
        d->bzfile = BZ2_bzWriteOpen(&bzerror, d->file, BlockSize100k, 0, 0);
    } else {
        fclose(d->file);
        return nullptr;
    }

    if (bzerror != BZ_OK) {
        fclose(d->file);
        return nullptr;
    }

    d->is_open = true;

    return this;
}

BZip2StreamBuf* BZip2StreamBuf::close()
{
    if (is_open()) {
        if ((pptr() != nullptr) && pptr() > pbase()) {
            flush_buffer();
        }
        d->is_open = false;

        int bzerror;
        if ((d->mode & std::ios::in) != 0) {
            BZ2_bzReadClose(&bzerror, d->bzfile);
        } else if ((d->mode & std::ios::out) != 0) {
            BZ2_bzWriteClose(&bzerror, d->bzfile, 0, nullptr, nullptr);
        } else {
            fclose(d->file);
            return nullptr;
        }

        fclose(d->file);
        if (bzerror == BZ_OK) {
            return this;
        }
    }

    return nullptr;
}

int BZip2StreamBuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) {
        return *reinterpret_cast<unsigned char*>(gptr());
    }

    if (((d->mode & std::ios::in) == 0) || !d->is_open) {
        return EOF;
    }

    // Josuttis' implementation of inbuf
    auto n_putback = 0L;
    if (gptr() != nullptr) {
        n_putback = std::min(gptr() - eback(), 4L);
        memcpy(d->buf.data() + (4 - n_putback), gptr() - n_putback, static_cast<std::size_t>(n_putback));
    }

    int bzerror;
    int num = BZ2_bzRead(&bzerror, d->bzfile, d->buf.data() + 4, BufferSize - 4);
    if (!(bzerror == BZ_OK || (bzerror == BZ_STREAM_END && num > 0))) {
        return EOF;
    }

    // reset buffer pointers
    setg(d->buf.data() + (4 - n_putback),  // beginning of putback area
         d->buf.data() + 4,                // read position
         d->buf.data() + 4 + num);         // end of buffer

    // return next character
    return *reinterpret_cast<unsigned char*>(gptr());
}

int BZip2StreamBuf::flush_buffer()
{
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    auto w = static_cast<int>(pptr() - pbase());
    int bzerror;
    BZ2_bzWrite(&bzerror, d->bzfile, pbase(), w);

    if (bzerror != BZ_OK) {
        return EOF;
    }

    pbump(-w);
    return w;
}

int BZip2StreamBuf::overflow(int c)
{
    if (((d->mode & std::ios::out) == 0) || !d->is_open) {
        return EOF;
    }
    if (c != EOF) {
        *pptr() = static_cast<char>(c);
        pbump(1);
    }
    if (flush_buffer() == EOF) {
        return EOF;
    }
    return c;
}

int BZip2StreamBuf::sync()
{
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ((pptr() != nullptr) && pptr() > pbase()) {
        if (flush_buffer() == EOF) {
            return -1;
        }
    }
    return 0;
}
}  // namespace fifr::util
