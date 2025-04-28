#include "Timer.hxx"

#include <iomanip>
#include <ostream>
#include <sstream>

#ifdef __unix
#include <sys/times.h>
#include <unistd.h>
#endif

namespace fifr::util {
#ifdef __unix
static const clock_t SC_CLK_TCK = sysconf(_SC_CLK_TCK);
#else
static const clock_t SC_CLK_TCK = CLOCKS_PER_SEC;
#endif

Timer::Timer(bool autostart)
{
    if (autostart) {
        start();
    }
}

Timer::~Timer()
{
    if (_running) {
        try {
            stop();
        } catch (...) {
        }
    }
}

void Timer::start(bool reset)
{
    if (reset || !_started) {
#ifdef __unix
        struct tms t {
        };
        if (times(&t) == -1) {
            throw std::runtime_error("Cannot get time, errno: " + std::to_string(errno));
        }
        _start_time = t.tms_utime;
        _started = true;
#else
        _start_time = clock();
#endif
    }
    _running = true;
}

void Timer::stop()
{
#ifdef __unix
    struct tms t {
    };
    if (times(&t) == -1) {
        throw std::runtime_error("Cannot get time, errno: " + std::to_string(errno));
    }
    _stop_time = t.tms_utime;
#else
    _stop_time = clock();
#endif
    _running = false;
}

double Timer::time() const
{
    return static_cast<double>(_stop_time - _start_time) / static_cast<double>(SC_CLK_TCK);
}

std::string Timer::str() const
{
    std::ostringstream s;
    s << *this;
    return s.str();
}

std::ostream& operator<<(std::ostream& o, const Timer& timer)
{
    if (!timer._started) {
        throw std::logic_error("Timer has not been started, yet");
    }
    auto diff = timer._stop_time - timer._start_time;
    if (timer._running) {
#ifdef __unix
        struct tms t {
        };
        if (times(&t) == -1) {
            throw std::runtime_error("Cannot get time, errno: " + std::to_string(errno));
        }
        diff = t.tms_utime - timer._start_time;
#else
        diff = clock() - timer._start_time;
#endif
    }
    auto h = diff / 3600 / SC_CLK_TCK;
    auto m = (diff / 60 / SC_CLK_TCK) % 60;
    auto s = (diff / SC_CLK_TCK) % 60;
    auto c = (diff * 100 / SC_CLK_TCK) % 100;

    o << std::setw(2) << std::setfill('0') << h << ":";
    o << std::setw(2) << std::setfill('0') << m << ":";
    o << std::setw(2) << std::setfill('0') << s << ".";
    o << std::setw(2) << std::setfill('0') << c;
    return o;
}

}  // namespace fifr::util
