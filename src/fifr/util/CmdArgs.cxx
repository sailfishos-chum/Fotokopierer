/*
 * Copyright (c) 2017, 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "CmdArgs.hxx"

#include "Join.hxx"
#include "Range.hxx"
#include "String.hxx"
#include "WordWrapStream.hxx"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <ostream>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace fifr::util {
struct CmdArgs::Data {
    std::string program_name;
    std::string synopsis;
    BaseOpt* help{};
    bool auto_shorts{};
    std::vector<std::string> stopon;
    std::vector<std::unique_ptr<BaseOpt>> opts;
    std::vector<std::unique_ptr<BaseOpt>> params;
    std::vector<std::string> rest;
    std::unique_ptr<CmdArgs::Section> next_section;
};

std::string CmdArgs::BaseOpt::to_string() const
{
    std::ostringstream out;
    if (!long_opt.empty()) {
        out << "--" << long_opt;
    } else if (short_opt != 0) {
        out << "-" << short_opt;
    }
    return out.str();
}

void CmdArgs::BaseOpt::see()
{
    if (seen) {
        throw CmdArgsError("Duplicated argument: " + to_string());
    }
    seen = true;
}

CmdArgs::Section::Section(std::string header) : header_(std::move(header)) {}

CmdArgs::Section::~Section() = default;

void CmdArgs::Section::parse_arg(const std::string&)
{
    assert(0);
}

bool CmdArgs::Section::has_arg() const
{
    assert(0);
    return false;
}

std::string CmdArgs::Section::default_string() const
{
    assert(0);
    return std::string();
}

CmdArgs::CmdArgs(const std::string& program_name) : d(new Data)
{
    d->program_name = program_name;
    d->help = nullptr;
    d->auto_shorts = true;
    d->stopon = {"--"};
}

CmdArgs::~CmdArgs() = default;

void CmdArgs::synopsis(const std::string& s)
{
    d->synopsis = s;
}

void CmdArgs::section(const std::string& s)
{
    assert(!s.empty());
    d->next_section = std::unique_ptr<CmdArgs::Section>(new Section(s));
}

void CmdArgs::no_auto_shorts()
{
    d->auto_shorts = false;
}

std::vector<std::string> CmdArgs::rest() const&
{
    return d->rest;
}

std::vector<std::string>&& CmdArgs::rest() &&
{
    return std::move(d->rest);
}

CmdArgs::BaseOpt* CmdArgs::add_opt(BaseOpt* opt)
{
    if (d->next_section) {
        d->opts.push_back(std::move(d->next_section));
        d->next_section = nullptr;
    }
    d->opts.push_back(std::unique_ptr<BaseOpt>(opt));
    return d->opts.back().get();
}

CmdArgs::BaseOpt* CmdArgs::add_param(BaseOpt* opt)
{
    if (d->next_section) {
        d->params.push_back(std::move(d->next_section));
        d->next_section = nullptr;
    }
    d->params.push_back(std::unique_ptr<BaseOpt>(opt));
    return d->params.back().get();
}

bool CmdArgs::parse_args(int argc, const char** argv)
{
    if (argc < 1) {
        throw CmdArgsError("Too few arguments");
    }

    d->program_name = argv[0];

    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (auto i : range(1, argc)) {
        args.emplace_back(argv[i]);
    }

    return parse(args);
}

bool CmdArgs::parse(const std::vector<std::string>& args)
{
    if (d->auto_shorts) {
        std::unordered_set<char> used_shorts;

        for (auto& arg : d->opts) {
            arg->seen = false;
            if (arg->short_opt != 0) {
                used_shorts.insert(arg->short_opt);
            }
        }

        for (auto& arg : d->opts) {
            if ((arg->short_opt == 0) && arg->auto_short) {
                for (auto c : arg->long_opt) {
                    if ((std::isalnum(c) != 0) && used_shorts.find(c) == used_shorts.end()) {
                        arg->short_opt = c;
                        used_shorts.insert(c);
                        break;
                    }
                }
            }
        }
    }

    std::size_t i = 0;
    std::size_t param_i = 0;
    while (i < args.size()) {
        if (std::find(d->stopon.begin(), d->stopon.end(), args[i]) != d->stopon.end()) {
            i += 1;
            break;
        }

        if (starts_with(args[i], "--")) {
            parse_long(args, i);
        } else if (args[i].size() > 1 && starts_with(args[i], "-")) {
            for (std::size_t j = 1; j < args[i].size(); ++j) {
                parse_short(args, i, j);
            }
        } else if (param_i < d->params.size()) {
            if (dynamic_cast<const Section*>(d->params[param_i].get()) != nullptr) {
                param_i += 1;
                continue;
            }
            d->params[param_i]->see();
            d->params[param_i]->parse_arg(args[i]);
            param_i += 1;
        } else {
            d->rest.push_back(args[i]);
        }
        i += 1;
    }

    d->rest.insert(d->rest.begin(), args.begin() + static_cast<std::ptrdiff_t>(i), args.end());

    if ((d->help != nullptr) && d->help->seen) {
        return false;
    }

    for (auto& p : d->params) {
        if (p->required && !p->seen) {
            throw CmdArgsError("Missing required parameter: <" + p->long_opt + ">");
        }
    }

    for (auto& opt : d->opts) {
        if (opt->required && opt->required_arg && !opt->seen) {
            throw CmdArgsError("Missing required option: " + opt->to_string());
        }
    }

    return true;
}

void CmdArgs::parse_long(const std::vector<std::string>& args, std::size_t& i)
{
    auto argpos = args[i].find('=');
    auto prefix = argpos != std::string::npos ? args[i].substr(2, argpos - 2) : args[i].substr(2);
    std::vector<BaseOpt*> candidates;
    bool perfect = false;

    for (auto& arg : d->opts) {
        if (starts_with(arg->long_opt, prefix)) {
            if (arg->long_opt.size() == prefix.size() && !perfect) {
                candidates.clear();
                candidates.push_back(arg.get());
                perfect = true;
            } else if (!perfect) {
                candidates.push_back(arg.get());
            }
        }
    }

    if (candidates.empty()) {
        throw CmdArgsError("Unknown option: " + prefix);
    }
    if (candidates.size() > 1) {
        std::vector<std::string> cands;
        cands.reserve(candidates.size());
        for (auto c : candidates) {
            cands.push_back(c->to_string());
        }
        throw CmdArgsError("Ambiguous prefix --" + prefix + " (candidates: " + join(cands, ", ") + ")");
    }

    if (argpos != std::string::npos) {
        if (!candidates[0]->has_arg()) {
            throw CmdArgsError("Unexpected argument for " + candidates[0]->to_string());
        }
        candidates[0]->parse_arg(args[i].substr(argpos + 1));
    } else if (candidates[0]->required || candidates[0]->required_arg) {
        if (i + 1 == args.size()) {
            throw CmdArgsError("Missing argument for " + candidates[0]->to_string());
        }
        candidates[0]->parse_arg(args[i + 1]);
        i += 1;
    }

    candidates[0]->see();
}

void CmdArgs::parse_short(const std::vector<std::string>& args, std::size_t& i, std::size_t& j)
{
    std::vector<BaseOpt*> candidates;

    for (auto& arg : d->opts) {
        if (arg->short_opt == args[i][j]) {
            candidates.push_back(arg.get());
        }
    }

    if (candidates.empty()) {
        throw CmdArgsError("Unknown option: -" + std::string(1, args[i][j]));
    }
    if (candidates.size() > 1) {
        throw CmdArgsError("Ambiguous options -" + std::string(1, args[i][j]));
    }

    if (candidates[0]->has_arg() && (candidates[0]->required || candidates[0]->required_arg)) {
        if (j + 1 < args[i].size()) {
            throw CmdArgsError("Short option -" + std::string(1, candidates[0]->short_opt) +
                               " with required argument must be last");
        }
        if (i + 1 == args.size()) {
            throw CmdArgsError("Missing argument for " + candidates[0]->to_string());
        }
        candidates[0]->parse_arg(args[i + 1]);
        i += 1;
        j = args[i].size();
    }
    candidates[0]->see();
}

void CmdArgs::add_help()
{
    d->help = &flag('h', "help").desc("Show this help page");
}

std::ostream& operator<<(std::ostream& out, const CmdArgs& args)
{
    out << "Usage: " << args.d->program_name << " [OPTIONS]";

    for (auto& p : args.d->params) {
        if (dynamic_cast<CmdArgs::Section*>(p.get()) != nullptr) {
            // skip section headers
        } else if (p->required) {
            out << " <" << p->long_opt << ">";
        } else {
            out << " [<" << p->long_opt << ">]";
        }
    }
    out << std::endl;

    if (!args.d->synopsis.empty()) {
        out << args.d->synopsis << std::endl;
    }

    if (!args.d->params.empty()) {
        if (dynamic_cast<CmdArgs::Section*>(args.d->params[0].get()) == nullptr) {
            // show default section header
            out << std::endl << "PARAMETER" << std::endl;
        }
        for (auto& p : args.d->params) {
            // possibly write section header
            auto s = dynamic_cast<CmdArgs::Section*>(p.get());
            if (s != nullptr) {
                out << std::endl << s->header() << std::endl;
                continue;
            }
            WordWrapStream wrapout(out);
            wrapout.set_indent_first(2);
            wrapout.set_indent(30);
            wrapout << "<" << p->long_opt << ">";
            wrapout.shift_to(28);
            wrapout << "  " << p->desc;
            if (p->has_default) {
                wrapout << " [default: " << p->default_string() << "]";
            }
            wrapout << "\n";
        }
    }

    if (!args.d->opts.empty()) {
        if (dynamic_cast<CmdArgs::Section*>(args.d->opts[0].get()) == nullptr) {
            // show default section header
            out << std::endl << "OPTIONS" << std::endl;
        }
        for (auto& opt : args.d->opts) {
            // possibly write section header
            auto s = dynamic_cast<CmdArgs::Section*>(opt.get());
            if (s != nullptr) {
                out << std::endl << s->header() << std::endl;
                continue;
            }
            WordWrapStream wrapout(out);
            wrapout.set_indent_first(2);
            wrapout.set_indent(30);
            if (opt->short_opt != 0) {
                wrapout << "-" << opt->short_opt;

                if (opt->has_arg() && opt->required_arg) {
                    wrapout << " <" << opt->arg_name() << ">";
                }

                if (!opt->long_opt.empty()) {
                    wrapout << ",";
                }
            }
            if (!opt->long_opt.empty()) {
                wrapout << "--" << opt->long_opt;

                if (opt->has_arg()) {
                    if (opt->required_arg) {
                        wrapout << "=<" << opt->arg_name() << ">";
                    } else {
                        wrapout << "[=<" << opt->arg_name() << ">]";
                    }
                }
            }

            wrapout.shift_to(28);
            wrapout << "  " << opt->desc;
            if (opt->has_default) {
                wrapout << " [default: " << opt->default_string() << "]";
            }
            wrapout << "\n";
        }
    }
    return out;
}
}  // namespace fifr::util
