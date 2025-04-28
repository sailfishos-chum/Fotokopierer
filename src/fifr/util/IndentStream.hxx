/*
 * Copyright (c) 2018 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_INDENTSTREAM_HXX__
#define __FIFR_UTIL_INDENTSTREAM_HXX__

#include <memory>
#include <ostream>

namespace fifr {
namespace util {
/// Steambuffer for the indentation stream.
///
/// Credits:
/// https://stackoverflow.com/questions/9599807/how-to-add-indention-to-the-stream-operator/9600752#9600752
class IndentStreamBuf : public std::streambuf
{
public:
    explicit IndentStreamBuf(std::ostream& out, std::size_t indent = 0, bool is_start_of_line = true)
        : out_(out), buf_(out.rdbuf()), is_start_of_line_(is_start_of_line), indent_(indent), default_inc_(2)
    {
    }

    IndentStreamBuf(IndentStreamBuf&&) = default;

    IndentStreamBuf(const IndentStreamBuf&) = delete;

    IndentStreamBuf& operator=(IndentStreamBuf&&) = delete;

    IndentStreamBuf& operator=(const IndentStreamBuf&) = delete;

    ~IndentStreamBuf() override { out_.rdbuf(buf_); }

    int overflow(int ch) override;

    void increase() { increase(default_inc_); }

    void increase(std::size_t inc) { indent_ += inc; }

    void decrease() { decrease(default_inc_); }

    void decrease(std::size_t dec)
    {
        if (dec <= indent_) {
            {
                {
                    indent_ -= dec;
                }
            }
        } else {
            {
                {
                    indent_ = 0;
                }
            }
        }
    }

    void set_default_inc(std::size_t default_inc) { default_inc_ = default_inc; }

private:
    std::ostream& out_;
    std::streambuf* buf_;
    bool is_start_of_line_;
    std::size_t indent_;
    std::size_t default_inc_;
};

class IndentStream : public std::ostream
{
public:
    explicit IndentStream(std::ostream& out, std::size_t indent = 0, bool is_start_of_line = true)
        : std::ostream(&buf_), buf_(out, indent, is_start_of_line)
    {
        if (out.width() > 0) {
            buf_.set_default_inc(static_cast<std::size_t>(out.width()));
        }
    }

    void increase() { buf_.increase(); }

    void increase(std::size_t inc) { buf_.increase(inc); }

    void decrease() { buf_.decrease(); }

    void decrease(std::size_t dec) { buf_.decrease(dec); }

private:
    IndentStreamBuf buf_;
};

}  // namespace util
}  // namespace fifr

#endif
