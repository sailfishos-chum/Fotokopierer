/*
 * Copyright (c) 2017, 2018 Frank Fischer <frank-fischer@shadow-soft.de>
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

#ifndef __FIFR_UTIL_CMDARGS_HXX__
#define __FIFR_UTIL_CMDARGS_HXX__

#include "Convert.hxx"

#include <exception>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace fifr {
namespace util {
template <typename T>
struct ArgType {
    static std::string name() { return typeid(T).name(); }

    static T parse(const std::string& arg) { return convert<T>(arg); }
};

#define FIFR_UTIL_DEF_ARGTYPE(typ, ctyp, name_)                                 \
    template <>                                                                 \
    struct ArgType<typ> {                                                       \
        static std::string name() { return #name_; }                            \
                                                                                \
        static typ parse(const std::string& arg) { return convert<ctyp>(arg); } \
    };

FIFR_UTIL_DEF_ARGTYPE(int8_t, int8_t, i8)
FIFR_UTIL_DEF_ARGTYPE(int16_t, int16_t, i16)
FIFR_UTIL_DEF_ARGTYPE(int32_t, int32_t, i32)
FIFR_UTIL_DEF_ARGTYPE(int64_t, int64_t, i64)
FIFR_UTIL_DEF_ARGTYPE(uint8_t, uint8_t, u8)
FIFR_UTIL_DEF_ARGTYPE(uint16_t, uint16_t, u16)
FIFR_UTIL_DEF_ARGTYPE(uint32_t, uint32_t, u32)
FIFR_UTIL_DEF_ARGTYPE(uint64_t, uint64_t, u64)

template <>
struct ArgType<std::string> {
    static std::string name() { return "s"; }

    static std::string parse(const std::string& arg) { return arg; }
};

class CmdArgsError : public std::exception
{
public:
    explicit CmdArgsError(std::string msg) : msg_(std::move(msg)) {}

    [[nodiscard]] const char* what() const noexcept override { return msg_.c_str(); }

private:
    std::string msg_;
};

class CmdArgs
{
private:
    class BaseOpt
    {
        friend class CmdArgs;
        friend std::ostream& operator<<(std::ostream& out, const CmdArgs& args);

    public:
        BaseOpt() = default;

        BaseOpt(BaseOpt&&) = delete;
        BaseOpt(const BaseOpt&) = delete;

        BaseOpt& operator=(BaseOpt&&) = delete;
        BaseOpt& operator=(const BaseOpt&) = delete;

        virtual ~BaseOpt() = default;

    protected:
        virtual void parse_arg(const std::string& opt) = 0;

        virtual std::string arg_name() { return "ARG"; }

        [[nodiscard]] virtual bool has_arg() const = 0;

        [[nodiscard]] virtual std::string default_string() const = 0;

        [[nodiscard]] std::string to_string() const;

        virtual void see();

    protected:
        std::string long_opt;
        bool auto_short = true;
        char short_opt = 0;
        std::string desc;
        bool required = false;
        bool required_arg = true;
        bool has_default = false;
        bool seen = false;
    };

    /// A special option representing a section header.
    class Section : public BaseOpt
    {
        friend class CmdArgs;

    private:
        explicit Section(std::string header);

    public:
        Section(Section&&) = delete;
        Section(const Section&) = delete;
        Section& operator=(Section&&) = delete;
        Section& operator=(const Section&) = delete;

        ~Section() override;

        [[nodiscard]] const std::string& header() const { return header_; }

    protected:
        void parse_arg(const std::string&) override;

        [[nodiscard]] bool has_arg() const override;

        [[nodiscard]] std::string default_string() const override;

    private:
        std::string header_;
    };

public:
    template <typename Self>
    class OptAdapter : public BaseOpt
    {
    public:
        /// Set the long version of the parameter.
        ///
        /// This sets the short option automatically unless an explicit
        /// short option is specified.
        Self& long_opt(std::string lng)
        {
            BaseOpt::long_opt = std::move(lng);
            return static_cast<Self&>(*this);
        }

        /// Set the short option.
        ///
        /// If set to 0, the argument does not have a short option
        /// (not even an automatic one).
        ///
        /// @see noshort
        Self& short_opt(char shrt)
        {
            auto_short = false;
            BaseOpt::short_opt = shrt;
            return static_cast<Self&>(*this);
        }

        /// Removes the short option.
        ///
        /// This is equivalent to `short_opt(0)`.
        Self& noshort() { return short_opt(0); }

        /// Sets the description string of this argument.
        Self& desc(std::string d)
        {
            BaseOpt::desc = std::move(d);
            return static_cast<Self&>(*this);
        }

        /// Return true iff this argument has been seen.
        [[nodiscard]] bool seen() const { return seen; }
    };

    template <typename T>
    class Opt : public OptAdapter<Opt<T>>
    {
        friend class CmdArgs;

    public:
        using value_type = T;

    protected:
        Opt() = default;

    public:
        /// Makes this an obligatory argument.
        Opt& required()
        {
            BaseOpt::required = true;
            return *this;
        }

        /// Set the default value.
        ///
        /// If `required_arg` is *false*, the option can be used without
        /// the argument.
        Opt& def(const T& default_value, bool required_arg = true)
        {
            value_ = default_value;
            BaseOpt::has_default = true;
            BaseOpt::required_arg = required_arg;
            return *this;
        }

        /// Return true iff this argument has been set.
        explicit operator bool() const { return BaseOpt::seen; }

        /// Return the value.
        [[nodiscard]] const T& value() const& { return value_; }

        /// Return the value.
        operator const T&() const& { return value(); }

        /// Return true iff the value of this argument equals the given value.
        bool operator==(const T& value) const { return value_ == value; }

        bool operator!=(const T& value) const { return !(*this == value); }

    protected:
        void parse_arg(const std::string& opt) override { value_ = ArgType<T>::parse(opt); }

        std::string arg_name() override { return ArgType<T>::name(); }

        [[nodiscard]] bool has_arg() const override { return true; }

        [[nodiscard]] std::string default_string() const override { return convert<std::string>(value_); }

    private:
        T value_ = {};
    };

    using Flag = Opt<bool>;

public:
    explicit CmdArgs(const std::string& program_name = "");

    CmdArgs(CmdArgs&&) = default;
    CmdArgs(const CmdArgs&) = delete;

    CmdArgs& operator=(CmdArgs&&) = default;
    CmdArgs& operator=(const CmdArgs&) = delete;

    ~CmdArgs();

    /// Set a short description.
    void synopsis(const std::string& s);

    /// Add a new section header.
    ///
    /// The section header is shown before the options and flags that
    /// are added after the header.
    void section(const std::string& s);

    /// Disable auto shorts.
    void no_auto_shorts();

    /// Add a new command line argument.
    template <typename T>
    Opt<T>& opt()
    {
        return opt<T>(0, {});
    }

    /// Add a new command line argument with a short option.
    template <typename T>
    Opt<T>& opt(char short_opt)
    {
        return opt<T>(short_opt, {});
    }

    /// Add a new command line argument with a long option.
    template <typename T>
    Opt<T>& opt(std::string long_opt)
    {
        return opt<T>(0, std::move(long_opt));
    }

    /// Add a new command line argument with short and long option.
    template <typename T>
    Opt<T>& opt(char short_opt, std::string long_opt)
    {
        auto& arg = *static_cast<Opt<T>*>(add_opt(new Opt<T>));
        if (short_opt != 0) {
            arg.short_opt(short_opt);
        }
        arg.long_opt(std::move(long_opt));
        return arg;
    }

    /// Add a new command line flag.
    Flag& flag() { return flag(0, {}); }

    /// Add a new command line argument with a short option.
    Flag& flag(char short_opt) { return flag(short_opt, {}); }

    /// Add a new command line argument with a long option.
    Flag& flag(std::string long_opt) { return flag(0, std::move(long_opt)); }

    /// Add a new command line argument with short and long option.
    Flag& flag(char short_opt, std::string long_opt) { return opt<bool>(short_opt, std::move(long_opt)); }

    /// Add a new parameter.
    template <typename T>
    Opt<T>& param(std::string name)
    {
        auto& param = *static_cast<Opt<T>*>(add_param(new Opt<T>));
        param.long_opt(std::move(name));
        return param;
    }

    /// Adds a help option.
    void add_help();

    /// Parse a list of strings as command line arguments.
    bool parse(const std::vector<std::string>& args);

    /// Parse command line arguments from `main`.
    bool parse_args(int argc, const char** argv);

    /// Return the unparsed arguments.
    [[nodiscard]] std::vector<std::string> rest() const&;

    /// Return the unparsed arguments.
    std::vector<std::string>&& rest() &&;

    friend std::ostream& operator<<(std::ostream& out, const CmdArgs& args);

private:
    BaseOpt* add_opt(BaseOpt* opt);

    BaseOpt* add_param(BaseOpt* opt);

    void parse_long(const std::vector<std::string>& args, std::size_t& i);

    void parse_short(const std::vector<std::string>& args, std::size_t& i, std::size_t& j);

private:
    struct Data;
    std::unique_ptr<Data> d;
};

template <>
class CmdArgs::Opt<bool> : public CmdArgs::OptAdapter<Opt<bool>>
{
    friend class CmdArgs;

protected:
    Opt() { BaseOpt::required_arg = false; }

public:
    /// Return true iff this argument has been set.
    ///
    /// Not that this might be different from `seen` if the default
    /// value is *true*.
    operator bool() const { return value_; }

    /// Return the value.
    [[nodiscard]] const bool& value() const& { return value_; }

    /// Set the default value.
    Opt& def(bool default_value)
    {
        value_ = default_value;
        return *this;
    }

protected:
    void parse_arg(const std::string&) override {}

    [[nodiscard]] bool has_arg() const override { return false; }

    void see() override
    {
        BaseOpt::see();
        value_ = !value_;
    }

    [[nodiscard]] std::string default_string() const override { return std::to_string(static_cast<int>(value_)); }

private:
    bool value_{false};
};

template <class T>
std::ostream& operator<<(std::ostream& out, const CmdArgs::Opt<T>& arg)
{
    return out << arg.value();
}
}  // namespace util
}  // namespace fifr

#endif
