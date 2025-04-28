/*
 * Copyright (c) 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

#include "Csv.hxx"

#include <cmath>
#include <fstream>
#include <istream>

namespace fifr {
namespace util {
CsvWriter::CsvWriter(std::unique_ptr<std::ostream> out, Parameters params) : CsvWriter(*out.get(), std::move(params))
{
    owned_out_ = std::move(out);
}

CsvWriter::CsvWriter(std::ostream& out, Parameters params)
    : out_(out), params_(params), forbidden_({params.field_separator, params.record_separator, params.quote})
{
    if (ForbiddenQuote.find(params.quote) != std::string_view::npos) {
        throw Error("Character '" + std::to_string(params.quote) + "' not allowed as quotation marker");
    }
}

CsvWriter CsvWriter::write(const std::string& filename, Parameters params)
{
    std::unique_ptr<std::ostream> out(new std::ofstream(filename));
    if (!*out) {
        throw Error("Error writing to file " + filename);
    }
    return CsvWriter(std::move(out), params);
}

void CsvWriter::quote(std::string_view s)
{
    if (s.find_first_of(forbidden_) != std::string_view::npos) {
        out_ << params_.quote;
        std::string_view::size_type beg = 0;
        while (true) {
            auto end = s.find(params_.quote, beg);
            if (end != std::string_view::npos) {
                out_ << s.substr(beg, end - beg) << params_.quote << params_.quote;
                beg = end + 1;
            } else {
                out_ << s.substr(beg);
                break;
            }
        }
        out_ << params_.quote;
    } else {
        out_ << s;
    }
}

CsvReadError::CsvReadError(const std::string& msg, std::size_t line_nr)
    : runtime_error(line_nr == 0 ? msg : msg + " (line: " + std::to_string(line_nr) + ")"), line_nr_(line_nr)
{
}

CsvReader::CsvReader(std::unique_ptr<std::istream> in, Parameters params) : CsvReader(*in.get(), std::move(params))
{
    owned_in_ = std::move(in);
}

CsvReader::CsvReader(std::istream& in, Parameters params) : in_(&in), params_(params)
{
    if (params_.headers) {
        init_headers();
    }
}

std::optional<std::size_t> CsvReader::column_index(std::string_view header) const
{
    auto it = std::find(headers_.begin(), headers_.end(), header);
    if (it != headers_.end()) {
        return static_cast<std::size_t>(it - headers_.begin());
    } else {
        return {};
    }
}

void CsvReader::init_headers()
{
    if (get(headers_)) {
        // do not count the header as record
        num_records_--;
        field_pos_.clear();
        raw_.clear();
    }
}

CsvReader CsvReader::read(const std::string& filename, Parameters params)
{
    std::unique_ptr<std::istream> in(new std::ifstream(filename));
    if (!*in) {
        throw Error("Error reading from file " + filename);
    }
    return CsvReader{std::move(in), std::move(params)};
}

bool CsvReader::next_record()
{
    if (in_ == nullptr || in_->eof()) {
        return false;
    }

    raw_.clear();
    field_pos_.clear();

    auto line = line_;
    char c;
    std::string::size_type beg = 0;
    while (true) {
        in_->get(c);
        if (in_->eof()) {
            line++;
            break;
        }

        // TODO: is this correct on windows?
        if (c == '\n') {
            line++;
        }

        if (c == params_.quote) {
            while (true) {
                in_->get(c);
                if (in_->eof()) {
                    throw Error("Unclosed quotation", line);
                }
                if (c == '\n') {
                    line++;
                }
                if (c != params_.quote) {
                    raw_.push_back(c);
                } else {
                    in_->get(c);
                    if (in_->eof()) {
                        // end of text, quotation is closed
                        break;
                    } else if (c == params_.quote) {
                        // double quote -> read a single quote
                        raw_.push_back(c);
                    } else {
                        // not a quote -> quotation is closed
                        in_->unget();
                        break;
                    }
                }
            }
        } else if (c == params_.record_separator) {
            // Skip empty records
            if (raw_.empty()) continue;
            trim_whitespaces();
            field_pos_.emplace_back(beg, raw_.size());
            beg = raw_.size();
            break;
        } else if (c == params_.field_separator) {
            trim_whitespaces();
            field_pos_.emplace_back(beg, raw_.size());
            beg = raw_.size();
        } else if (params_.ignore_whitespace && beg == raw_.size() && std::isspace(c)) {
            // ignore the character
        } else {
            raw_.push_back(c);
        }
    }

    // The last field might have not been finished (missing trailing separator).
    // This happens if we reached the EOF but have already read at least one character
    // so that the current record is not empty.
    if (in_->eof() && (!raw_.empty() || !field_pos_.empty())) {
        trim_whitespaces();
        field_pos_.emplace_back(beg, raw_.size());
    }

    // If we did not read a record we must have reached the EOF.
    if (field_pos_.empty()) {
        in_ = nullptr;
        owned_in_ = nullptr;
        return false;
    }

    if (params_.num_columns != 0 && params_.num_columns != field_pos_.size()) {
        throw Error("Invalid number of columns (expected: " + std::to_string(params_.num_columns) +
                        " got: " + std::to_string(field_pos_.size()) + ")",
                    line);
    }

    min_columns_ = std::min(min_columns_, field_pos_.size());
    max_columns_ = std::max(max_columns_, field_pos_.size());
    line_ = line;
    num_records_++;

    return true;
}

void CsvReader::trim_whitespaces()
{
    if (params_.ignore_whitespace) {
        while (!raw_.empty() && std::isspace(raw_.back())) {
            raw_.pop_back();
        }
    }
}

template bool CsvReader::get(std::vector<std::string>& x);
template bool CsvReader::get(std::vector<std::string_view>& x);

}  // namespace util
}  // namespace fifr
