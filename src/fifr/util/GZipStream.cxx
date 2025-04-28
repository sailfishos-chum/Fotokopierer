#include "GZipStream.hxx"

#include <zlib.h>

#include <array>
#include <cstring>

namespace fifr::util {
static const unsigned BufferSize = 47 + 256;

struct GZipStreamBuf::Data {
    gzFile file = nullptr;
    std::array<char, BufferSize> buf = {};
    bool is_open = false;
    std::ios::openmode mode = {};
};

GZipStreamBuf::GZipStreamBuf() : d(new Data)
{
    setp(d->buf.data(), d->buf.data() + (BufferSize - 1));
    setg(d->buf.data() + 4, d->buf.data() + 4, d->buf.data() + 4);
}

GZipStreamBuf::~GZipStreamBuf()
{
    close();
}

bool GZipStreamBuf::is_open() const
{
    return d->is_open;
}

GZipStreamBuf* GZipStreamBuf::open(const std::string& name, std::ios::openmode op_mode)
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

    d->file = gzopen(name.c_str(), &fmode[0]);
    if (d->file == nullptr) {
        return nullptr;
    }

    d->is_open = true;

    return this;
}

GZipStreamBuf* GZipStreamBuf::close()
{
    if (is_open()) {
        if ((pptr() != nullptr) && pptr() > pbase()) {
            flush_buffer();
        }
        d->is_open = false;
        if (gzclose(d->file) == Z_OK) {
            return this;
        }
    }
    return nullptr;
}

int GZipStreamBuf::underflow()  // used for input buffer only
{
    if ((gptr() != nullptr) && (gptr() < egptr())) {
        return *reinterpret_cast<unsigned char*>(gptr());  // NOLINT
    }

    if (((d->mode & std::ios::in) == 0) || !d->is_open) {
        return EOF;
    }

    // Josuttis' implementation of inbuf
    auto n_putback = 0L;
    if (gptr() != nullptr) {
        n_putback = std::min(4L, gptr() - eback());
        memcpy(d->buf.data() + (4 - n_putback),
               gptr() - n_putback,  // NOLINT
               static_cast<std::size_t>(n_putback));
    }

    int num = gzread(d->file, d->buf.data() + 4, BufferSize - 4);
    if (num <= 0) {  // ERROR or EOF
        return EOF;
    }

    // reset buffer pointers
    setg(d->buf.data() + (4 - n_putback),  // beginning of putback area
         d->buf.data() + 4,                // read position
         d->buf.data() + 4 + num);         // end of buffer

    // return next character
    return *reinterpret_cast<unsigned char*>(gptr());  // NOLINT
}

int GZipStreamBuf::flush_buffer()
{
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    auto w = static_cast<int>(pptr() - pbase());
    if (gzwrite(d->file, pbase(), static_cast<unsigned>(w)) != w) {
        return EOF;
    }
    pbump(-w);
    return w;
}

int GZipStreamBuf::overflow(int c)  // used for output buffer only
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

int GZipStreamBuf::sync()
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
