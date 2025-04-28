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

#include "ToolStream.hxx"

#include "Join.hxx"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>

using namespace __gnu_cxx;
using namespace std::literals;

namespace fifr::util {
namespace {
int open_tool(const std::string& command,
              const std::vector<std::string>& args,
              const std::string& filename,
              std::ios_base::openmode mode,
              pid_t& pid)
{
    char err[128];
    int fd[2];

    if (mode != std::ios::in && mode != std::ios::out) {
        throw std::runtime_error("Only std::ios::in and std::ios::out are supported");
    }

    if (pipe(fd) == -1) {
        throw std::runtime_error("Cannot create pipes: "s + strerror_r(errno, err, 128));
    }

    pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Cannot fork process pipes: "s + strerror_r(errno, err, 128));
    }

    if (pid == 0) {
        if (mode == std::ios::in) {
            close(fd[0]);
            if (dup2(fd[1], 1) == -1) {
                std::cerr << "Cannot redirect stdout: " << strerror_r(errno, err, 128) << std::endl;
                std::exit(EXIT_FAILURE);
            };

            if (!filename.empty()) {
                auto file = open(filename.c_str(), O_RDONLY);  // NOLINT
                if (file == -1) {
                    std::cerr << "Cannot open file " << filename << ": " << strerror_r(errno, err, 128) << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                if (dup2(file, 0) == -1) {
                    std::cerr << "Cannot redirect input from file: " << strerror_r(errno, err, 128) << std::endl;
                    std::exit(EXIT_FAILURE);
                };
            }
        } else {
            close(fd[1]);
            if (dup2(fd[0], 0) == -1) {
                std::cerr << "Cannot redirect stdin: " << strerror_r(errno, err, 128) << std::endl;
                std::exit(EXIT_FAILURE);
            }

            if (!filename.empty()) {
                auto file = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  // NOLINT
                if (file == -1) {
                    std::cerr << "Cannot write to file " << filename << ": " << strerror_r(errno, err, 128) << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                if (dup2(file, 1) == -1) {
                    std::cerr << "Cannot redirect output to file: " << strerror_r(errno, err, 128) << std::endl;
                    std::exit(EXIT_FAILURE);
                };
            }
        }

        std::vector<char*> argp(args.size() + 2, nullptr);
        argp[0] = const_cast<char*>(command.c_str());  // NOLINT
        std::transform(args.begin(), args.end(), argp.begin() + 1, [](auto& arg) {
            return const_cast<char*>(arg.c_str());  // NOLINT
        });
        char* const* envp = {nullptr};

        if (execve(command.c_str(), &argp[0], envp) == -1) {
            std::cerr << "Cannot execute '" << command << " " << join(args, " ") << "': " << strerror_r(errno, err, 128)
                      << std::endl;
        }
        std::exit(EXIT_FAILURE);
    }

    if (mode == std::ios::out) {
        std::swap(fd[0], fd[1]);
    }

    close(fd[1]);

    auto flags = fcntl(fd[0], F_GETFD);  // NOLINT
    if (flags == -1) {
        throw std::runtime_error("Error calling fcntl: "s + strerror_r(errno, err, 128));
    }

    flags |= FD_CLOEXEC;
    if (fcntl(fd[0], F_SETFD, flags) == -1) {  // NOLINT
        throw std::runtime_error("Error calling fcntl: "s + strerror_r(errno, err, 128));
    }

    return fd[0];
}

}  // namespace

ToolStreambuf::ToolStreambuf(const std::string& command, const std::vector<std::string>& args, std::ios_base::openmode mode)
    : stdio_filebuf(open_tool(command, args, {}, mode, pid_), mode)
{
}

ToolStreambuf::ToolStreambuf(const std::string& command,
                             const std::vector<std::string>& args,
                             const std::string& filename,
                             std::ios_base::openmode mode)
    : stdio_filebuf(open_tool(command, args, filename, mode, pid_), mode)
{
}

ToolStreambuf::ToolStreambuf(ToolStreambuf&& b) noexcept : stdio_filebuf(std::move(b)), pid_(b.pid_)
{
    b.pid_ = 0;
}

ToolStreambuf& ToolStreambuf::operator=(ToolStreambuf&& b) noexcept
{
    close();
    std::swap(pid_, b.pid_);
    stdio_filebuf::operator=(std::move(b));
    return *this;
}

ToolStreambuf::~ToolStreambuf()
{
    close();
}

void ToolStreambuf::close()
{
    if (is_open() && pid_ != 0) {
        int status;
        stdio_filebuf::close();
        waitpid(pid_, &status, 0);
    }
    pid_ = 0;
}

InputToolStream::InputToolStream(const std::string& command,
                                 const std::vector<std::string>& args,
                                 const std::string& filename)
    : std::istream(&buf_), buf_(command, args, filename, std::ios::in)
{
}

InputToolStream::InputToolStream(InputToolStream&& s) noexcept : std::istream(std::move(s)), buf_(std::move(s.buf_))
{
    std::istream::set_rdbuf(&buf_);
}

void InputToolStream::open(const std::string& command, const std::vector<std::string>& args, const std::string& filename)
{
    buf_ = ToolStreambuf(command, args, filename, std::ios::in);
    if (buf_.is_open()) {
        clear();
    } else {
        setstate(std::ios_base::failbit);
    }
}

InputToolStream& InputToolStream::operator=(InputToolStream&& s) noexcept
{
    buf_ = std::move(s.buf_);
    std::istream::operator=(std::move(s));
    return *this;
}

InputToolStream::~InputToolStream() = default;

OutputToolStream::OutputToolStream(const std::string& command,
                                   const std::vector<std::string>& args,
                                   const std::string& filename)
    : std::ostream(&buf_), buf_(command, args, filename, std::ios::out)
{
}

OutputToolStream::OutputToolStream(OutputToolStream&& s) noexcept : std::ostream(std::move(s)), buf_(std::move(s.buf_))
{
    std::ostream::set_rdbuf(&buf_);
}

OutputToolStream& OutputToolStream::operator=(OutputToolStream&& s) noexcept
{
    buf_ = std::move(s.buf_);
    std::ostream::operator=(std::move(s));
    return *this;
}

OutputToolStream::~OutputToolStream() = default;

void OutputToolStream::open(const std::string& command, const std::vector<std::string>& args, const std::string& filename)
{
    buf_ = ToolStreambuf(command, args, filename, std::ios::out);
    if (buf_.is_open()) {
        clear();
    } else {
        setstate(std::ios_base::failbit);
    }
}

}  // namespace fifr::util
