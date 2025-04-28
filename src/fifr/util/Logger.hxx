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

#include <ostream>

namespace fifr {
namespace util {
/// A simple Logger.
class Logger
{
public:
    class LogStream
    {
    public:
        LogStream(const char* name, std::ostream* s) : stream_(s)
        {
            if (s != nullptr) {
                *s << name << " ";
            }
        }

        ~LogStream()
        {
            if (stream_ != nullptr) {
                *stream_ << std::endl;
            }
        }

        LogStream(LogStream&&) = default;

        LogStream(const LogStream&) = delete;

        LogStream& operator=(LogStream&&) = default;

        LogStream& operator=(const LogStream&) = delete;

        std::ostream* stream() { return stream_; }

    private:
        std::ostream* stream_;
    };

public:
    enum class Level {
        Debug,  ///< Show debug messages and everything else.
        Info,   ///< Show info, warning and error messages.
        Warn,   ///< Show warning and error messages.
        Error,  ///< Show error messages.
        None,   ///< Show nothing.
    };

public:
    explicit Logger(std::ostream& out) noexcept : out_(out), level_(Level::Info) {}

    /// Set the log level.
    void set_level(Level level) { level_ = level; }

    /// Return current log level.
    [[nodiscard]] Level level() const { return level_; }

    /// Return info log stream.
    LogStream info() { return {"I  ", level_ <= Level::Info ? &out_ : nullptr}; }

    /// Return warning log stream.
    LogStream warn() { return {"W  ", level_ <= Level::Warn ? &out_ : nullptr}; }

    /// Return error log stream.
    LogStream error() { return {"E  ", level_ <= Level::Error ? &out_ : nullptr}; }

    /// Return debug log stream.
    LogStream debug() { return {"D  ", level_ <= Level::Debug ? &out_ : nullptr}; }

private:
    /// Default logger, logs to stderr.
    Logger() noexcept;

private:
    std::ostream& out_;
    Level level_;

public:
    /// The global default logger.
    static Logger default_logger;
};

/// Return info stream for the default logger.
inline Logger::LogStream info()
{
    return Logger::default_logger.info();
}

/// Return warning stream for the default logger.
inline Logger::LogStream warn()
{
    return Logger::default_logger.warn();
}

/// Return error stream for the default logger.
inline Logger::LogStream error()
{
    return Logger::default_logger.error();
}

/// Return debug stream for the default logger.
inline Logger::LogStream debug()
{
    return Logger::default_logger.debug();
}

/// Write some value to a log stream.
template <class T>
Logger::LogStream& operator<<(Logger::LogStream& log, const T& x)
{
    auto s = log.stream();
    if (s) {
        *s << x;
    }
    return log;
}

/// Write some value to a log stream.
template <class T>
Logger::LogStream& operator<<(Logger::LogStream&& log, const T& x)
{
    auto s = log.stream();
    if (s) {
        *s << x;
    }
    return log;
}
}  // namespace util
}  // namespace fifr
