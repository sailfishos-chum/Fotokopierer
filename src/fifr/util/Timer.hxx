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

#ifndef __FIFR_UTIL_TIMER_HXX__
#define __FIFR_UTIL_TIMER_HXX__

#include <ctime>
#include <iosfwd>
#include <string>

namespace fifr {
namespace util {
/// A simple timer class.
class Timer
{
public:
    /// Create a new timer.
    ///
    /// If `autostart` is *true* the timer is started on creation.
    explicit Timer(bool autostart = false);

    Timer(const Timer&) = default;
    Timer(Timer&&) = default;
    Timer& operator=(const Timer&) = default;
    Timer& operator=(Timer&&) = default;

    ~Timer();

    /// Start the timer.
    ///
    /// If `restart` is `true` the timer is reset to 0.
    void start(bool reset = false);

    /// Stop the timer.
    void stop();

    /// Return the elapsed seconds.
    [[nodiscard]] double time() const;

    /// Return a string representation of time.
    [[nodiscard]] std::string str() const;

    friend std::ostream& operator<<(std::ostream& o, const Timer& timer);

private:
    bool _running{false};
    clock_t _start_time{0}, _stop_time{0};
    bool _started{false};
};

std::ostream& operator<<(std::ostream& o, const Timer& timer);

}  // namespace util
}  // namespace fifr

#endif
