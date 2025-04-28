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

#ifndef __FIFR_UTIL_TOOLSTREAM_HXX__
#define __FIFR_UTIL_TOOLSTREAM_HXX__

#include <ext/stdio_filebuf.h>
#include <sys/types.h>

#include <string>
#include <vector>

namespace fifr {
namespace util {
/// A stream-buffer for piping data through an external tool.
class ToolStreambuf : public __gnu_cxx::stdio_filebuf<char>
{
public:
    /// Create a streambuffer piping through the given command.
    ToolStreambuf(const std::string& command, const std::vector<std::string>& args, std::ios_base::openmode mode);

    /// Create a streambuffer piping through the given command.
    ///
    /// The output/input of the command is written to/read from the given file.
    ToolStreambuf(const std::string& command,
                  const std::vector<std::string>& args,
                  const std::string& filename,
                  std::ios_base::openmode mode);

    ToolStreambuf(ToolStreambuf&&) noexcept;
    ToolStreambuf(const ToolStreambuf&) = delete;
    ToolStreambuf& operator=(ToolStreambuf&&) noexcept;
    ToolStreambuf& operator=(const ToolStreambuf&) = delete;

    ~ToolStreambuf() override;

    void close();

private:
    pid_t pid_;
};

/// An input stream that gets its data from an external tool.
class InputToolStream : public std::istream
{
public:
    explicit InputToolStream(const std::string& command, const std::vector<std::string>& args = {})
        : InputToolStream(command, args, {})
    {
    }

    InputToolStream(const std::string& command, const std::vector<std::string>& args, const std::string& filename);

    InputToolStream(InputToolStream&& s) noexcept;

    InputToolStream(const InputToolStream&) = delete;

    ~InputToolStream() override;

    InputToolStream& operator=(InputToolStream&& s) noexcept;

    InputToolStream& operator=(const InputToolStream&) = delete;

    void open(const std::string& command, const std::vector<std::string>& args = {}) { open(command, args, {}); }

    void open(const std::string& command, const std::vector<std::string>& args, const std::string& filename);

    void close()
    {
        if (buf_.is_open()) {
            buf_.close();
            clear();
        }
    }

private:
    ToolStreambuf buf_;
};

/// An output stream that pipes its data through an external tool.
class OutputToolStream : public std::ostream
{
public:
    explicit OutputToolStream(const std::string& command, const std::vector<std::string>& args = {})
        : OutputToolStream(command, args, {})
    {
    }

    OutputToolStream(const std::string& command, const std::vector<std::string>& args, const std::string& filename);

    OutputToolStream(OutputToolStream&& s) noexcept;

    OutputToolStream(const OutputToolStream&) = delete;

    ~OutputToolStream() override;

    OutputToolStream& operator=(OutputToolStream&& s) noexcept;

    OutputToolStream& operator=(const OutputToolStream&) = delete;

    void open(const std::string& command, const std::vector<std::string>& args = {}) { open(command, args, {}); }

    void open(const std::string& command, const std::vector<std::string>& args, const std::string& filename);

    void close()
    {
        if (buf_.is_open()) {
            buf_.close();
            clear();
        }
    }

private:
    ToolStreambuf buf_;
};

using itoolstream = InputToolStream;
using otoolstream = OutputToolStream;

}  // namespace util
}  // namespace fifr

#endif
