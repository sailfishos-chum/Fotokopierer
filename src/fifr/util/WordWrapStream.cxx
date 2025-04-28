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

#include "WordWrapStream.hxx"

#include <cassert>
#include <limits>
#include <sstream>

namespace fifr::util {
namespace {
enum class Line { First, Newline, Mid };
}

struct WordWrapStream::Data {
    explicit Data(std::ostream& o) : out(o) {}

    std::ostream& out;
    std::size_t width = DEFAULT_WIDTH;
    std::size_t indent = 0;
    std::size_t indent_first = std::numeric_limits<std::size_t>::max();
    std::ostringstream buffer;

    Line line = Line::First;
    std::size_t pos = 0;
};

WordWrapStream::WordWrapStream(std::ostream& out) : d(new Data(out)) {}

WordWrapStream::~WordWrapStream()
{
    flush();
}

void WordWrapStream::set_width(std::size_t width)
{
    assert(width > 0);
    d->width = width;
}

void WordWrapStream::set_indent(std::size_t indent)
{
    d->indent = indent;
}

void WordWrapStream::set_indent_first(std::size_t indent)
{
    d->indent_first = indent;
}

void WordWrapStream::flush()
{
    std::string str = d->buffer.str();
    d->buffer.str("");

    std::string::size_type beg = 0;

    while (beg < str.size()) {
        if (d->line == Line::Mid) {
            auto mid = str.find_first_not_of(" \t\r", beg);
            auto end = str.find_first_of(" \t\r\n", mid);
            if (end == std::string::npos) {
                end = str.size();
            }
            if (d->pos + end - beg > d->width) {
                d->line = Line::Newline;
                beg = mid;
            } else if (end != std::string::npos && str[end] == '\n') {
                d->out << str.substr(beg, end - beg);
                beg = end;
                d->line = Line::Newline;
                // skip line break because we add one anyway
                if (beg + 1 < str.size()) {
                    beg += 1;
                } else {
                    d->out << std::endl;
                    break;
                }
            } else {
                d->out << str.substr(beg, end - beg);
                d->pos += end - beg;
                beg = end;
            }
        } else {
            std::size_t indent = d->indent;
            if (d->line == Line::First) {
                if (d->indent_first != std::numeric_limits<std::size_t>::max()) {
                    indent = d->indent_first;
                }
            } else {
                d->out << std::endl;
            }
            for (std::size_t i = 0; i < indent; i++) {
                d->out.put(' ');
            }
            d->pos = std::min(d->width - 1, indent);
            d->line = Line::Mid;
        }
    }
}

void WordWrapStream::shift_to(std::size_t pos)
{
    flush();
    if (d->line != Line::Mid) {
        d->line = Line::Mid;
        d->out << std::endl;
        d->pos = 0;
    }

    if (d->pos < pos) {
        for (std::size_t i = d->pos; i < pos; i++) {
            d->out.put(' ');
        }
        d->pos = pos;
    }
}

std::ostream& WordWrapStream::stream()
{
    return d->buffer;
}

const std::size_t WordWrapStream::DEFAULT_WIDTH;
}  // namespace fifr::util
