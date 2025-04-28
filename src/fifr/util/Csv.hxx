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

#ifndef __FIFR_UTIL_CSV_HXX__
#define __FIFR_UTIL_CSV_HXX__

#include "Convert.hxx"
#include "Range.hxx"

#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace fifr {
namespace util {
/// A error when writing a CSV file occurred.
class CsvWriteError : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/// Parameters used for writing.
struct CsvWriteParameters {
    /// The field separator, defaults to ","
    char field_separator = ',';

    /// The record separator, defaults to "\n"
    char record_separator = '\n';

    /// The quotation character.
    char quote = '"';

    /// Require that all records have the numbers of fields.
    std::size_t fixed_columns = true;
};

class CsvWriter
{
public:
    /// Exception class raised on error.
    using Error = CsvWriteError;

    /// Parameters to configurate the writer.
    using Parameters = CsvWriteParameters;

    /// List of characters that must not be used a quote character.
    static constexpr std::string_view ForbiddenQuote = "0123456789eE.+-";

public:
    /// Create `CsvWriter` writing to an output stream and taking ownership.
    explicit CsvWriter(std::unique_ptr<std::ostream> in, Parameters params = {});

    /// Create `CsvWriter` writing to an output stream and without taking ownership.
    explicit CsvWriter(std::ostream& in, Parameters params = {});

    /// Write a vector as a new record.
    template <typename T>
    void put(const std::vector<T>& fields)
    {
        update_num_columns(fields.size());
        bool first = true;
        for (auto& f : fields) {
            if (!first)
                out_ << params_.field_separator;
            else
                first = false;
            quote(f);
        }
        out_ << params_.record_separator;
        if (!out_) throw Error("Error writing csv file");
    }

    /// Write an array as a new record.
    template <typename T, std::size_t N>
    void put(const std::array<T, N>& fields)
    {
        put_impl(fields, std::make_index_sequence<N>{});
    }

    /// Write a tuple as a new record.
    template <typename... Args>
    void put(const std::tuple<Args...>& fields)
    {
        put_impl(fields, std::make_index_sequence<sizeof...(Args)>{});
    }

    /// Write several arguments as a new record.
    template <typename... Args>
    void put(Args... args)
    {
        update_num_columns(sizeof...(Args));
        put_impl(args...);
    }

    /// Output stream interface for writing record.
    template <typename T>
    CsvWriter& operator<<(T&& fields)
    {
        put(std::forward<T>(fields));
        return *this;
    }

    /// Return a CsvWriter open for writing to an output stream.
    static CsvWriter write(std::ostream& out, Parameters params = {}) { return CsvWriter(out, params); }

    /// Return a CsvWriter open for writing to a file.
    static CsvWriter write(const std::string& filename, Parameters params = {});

private:
    template <typename T, std::size_t N, std::size_t... I>
    void put_impl(const std::array<T, N>& fields, std::index_sequence<I...>)
    {
        update_num_columns(N);
        put_impl(fields[I]...);
    }

    template <typename... Args, std::size_t... I>
    void put_impl(const std::tuple<Args...>& fields, std::index_sequence<I...>)
    {
        update_num_columns(sizeof...(Args));
        put_impl(std::get<I>(fields)...);
    }

    template <typename Arg0>
    void put_impl(Arg0 arg0)
    {
        quote(arg0);
        out_ << params_.record_separator;
        if (!out_) {
            throw Error("Error writing csv file");
        }
    }

    template <typename Arg0, typename Arg1, typename... Args>
    void put_impl(Arg0 arg0, Arg1 arg1, Args... args)
    {
        quote(arg0);
        out_ << params_.field_separator;
        if (!out_) {
            throw Error("Error writing csv file");
        }
        put_impl(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }

    /// Write a string to an output stream, possibly quoting the text.
    ///
    /// \param quote is the quotation character
    /// \param forbidden is the set characters that must be quoted
    /// \param x the string to be written
    void quote(std::string_view s);

    /// \overload
    template <typename T, std::enable_if_t<std::is_convertible<T, std::string_view>::value, int> = 0>
    void quote(const T& x)
    {
        return quote(std::string_view{x});
    }

    /// Write an element to an output stream, possibly quoting the text.
    ///
    /// \param quote is the quotation character
    /// \param forbidden is the set characters that must be quoted
    /// \param x the element to be written
    template <typename T,
              std::enable_if_t<!std::is_arithmetic<T>::value && !std::is_convertible<T, std::string_view>::value, int> = 0>
    void quote(const T& x)
    {
        quote(std::string_view{convert<std::string>(x)});
    }

    /// \overload
    template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
    void quote(const T& x)
    {
        out_ << x;
    }

    void update_num_columns(std::size_t n)
    {
        if (params_.fixed_columns) {
            if (num_columns_ != n) {
                if (num_columns_ == 0) {
                    num_columns_ = n;
                } else {
                    throw Error("Records have different numbers of fields (" + std::to_string(n) + ", " +
                                std::to_string(num_columns_) + ")");
                }
            }
        }
    }

private:
    std::unique_ptr<std::ostream> owned_out_ = nullptr;
    std::ostream& out_;

    Parameters params_;
    std::string forbidden_;

    std::size_t num_columns_ = 0;
};

/// An error occurred during reading a CSV file.
class CsvReadError : public std::runtime_error
{
public:
    explicit CsvReadError(const std::string& msg, std::size_t line_nr = 0);

    std::size_t line_number() const { return line_nr_; }

private:
    std::size_t line_nr_;
};

/// Parameters for reading a CSV file.
struct CsvReaderParameters {
    /// The field separator, defaults to ","
    char field_separator = ',';

    /// The record separator, defaults to "\n"
    char record_separator = '\n';

    /// Ignore whitespace at the beginning and end of a field.
    ///
    /// If one of the separators is a whitespace character,
    /// this character is not ignored.
    bool ignore_whitespace = false;

    /// The quotation character.
    ///
    /// The default quotation character is ".
    /// All text between "..." is taken literally. To include a single " within
    /// a quoted text, double the character: "text""text".
    char quote = '"';

    /// The first line contains column headers.
    bool headers = false;

    /// The exact number of columns (0 means record may have different numbers of
    /// columns).
    std::size_t num_columns = 0;
};

/// A range of CSV records.
class CsvReader
{
    friend class iterator;

public:
    /// Exception raised on error.
    using Error = CsvReadError;

    /// Parameters to configurate the reader.
    using Parameters = CsvReaderParameters;

    template <std::size_t N = 0, typename Fields = std::vector<std::string_view>>
    class RecordIterator
    {
    private:
        static constexpr auto NumColumns = N;

    public:
        using value_type = Fields;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        RecordIterator() : range_(nullptr) {}

        explicit RecordIterator(CsvReader& range, const std::array<std::size_t, N>& inds = {})
            : range_(&range), inds_(std::move(inds))
        {
            if (range.field_pos_.empty()) {
                // no record has been read so far, just read the first one.
                ++(*this);
            }
        }

        RecordIterator(RecordIterator&) = delete;
        RecordIterator(RecordIterator&&) = default;
        RecordIterator& operator=(RecordIterator&) = delete;
        RecordIterator& operator=(RecordIterator&&) = default;

        reference operator*()
        {
            assert(range_);
            return fields_;
        }

        pointer operator->()
        {
            assert(range_);
            return &fields_;
        }

        RecordIterator& operator++()
        {
            assert(range_);
            if (!range_->get_tuple(inds_, fields_)) {
                range_ = nullptr;
            }
            return *this;
        }

        bool operator==(const RecordIterator& it) const { return range_ == it.range_; }
        bool operator!=(const RecordIterator& it) const { return !(*this == it); }

    private:
        CsvReader* range_;
        Fields fields_;
        std::array<std::size_t, N> inds_;
    };

    /// The default, untyped iterator.
    using iterator = RecordIterator<>;

    /// A wrapper around named columns.
    template <std::size_t N, typename Fields>
    class Named
    {
    public:
        static constexpr auto NumColumns = N;

        using iterator = RecordIterator<NumColumns, Fields>;

    public:
        Named(CsvReader& csv, const std::array<std::string_view, NumColumns>& columns) : csv_(csv)
        {
            compute_column_indices(columns);
        }

    private:
        void compute_column_indices(const std::array<std::string_view, NumColumns>& columns);

    public:
        iterator begin() { return iterator(csv_, indices_); }

        iterator end() { return {}; }

    private:
        CsvReader& csv_;
        std::array<std::size_t, NumColumns> indices_;
    };

public:
    /// Create `CsvReader` reading from an input stream and taking ownership.
    explicit CsvReader(std::unique_ptr<std::istream> in, Parameters params = {});

    /// Create `CsvReader` reading from an input stream and without taking ownership.
    explicit CsvReader(std::istream& in, Parameters params = {});

    /// Returns the number of records read so far.
    std::size_t num_records() const { return num_records_; }

    /// Returns the maximal number of columns seen in the file.
    std::size_t max_columns() const { return max_columns_; }

    /// Returns the minimal number of columns seen in the file.
    std::size_t min_columns() const { return min_columns_; }

    /// Returns the number of lines read so far.
    std::size_t num_lines() const { return line_; }

    /// Return the header of the given column.
    ///
    /// If the column does not have a header, an empty string is returned.
    std::string_view header(std::size_t i) const { return i < headers_.size() ? headers_[i] : std::string_view{}; }

    /// Return the index of the column with the given header.
    ///
    /// The returned value is invalid if no column with this header exists.
    std::optional<std::size_t> column_index(std::string_view header) const;

    iterator begin() { return iterator{*this}; }

    iterator end() { return {}; }

    template <typename T>
    Named<0, std::vector<T>> rows()
    {
        return Named<0, std::vector<T>>{*this, {}};
    }

    template <typename T, std::size_t N>
    Named<0, std::array<T, N>> rows()
    {
        return Named<0, std::array<T, N>>{*this, {}};
    }

    template <typename Arg0, typename Arg1, typename... Args>
    Named<0, std::tuple<Arg0, Arg1, Args...>> rows()
    {
        return Named<0, std::tuple<Arg0, Arg1, Args...>>{*this, {}};
    }

    template <typename... Columns>
    Named<sizeof...(Columns), std::array<std::string_view, sizeof...(Columns)>> named(Columns... columns)
    {
        if (params_.headers == false && headers_.empty()) {
            init_headers();
        }
        return Named<sizeof...(Columns), std::array<std::string_view, sizeof...(Columns)>>{
            *this, std::array{std::string_view{columns}...}};
    }

    template <typename... Args>
    Named<sizeof...(Args), std::tuple<Args...>> named(typename std::conditional<false, Args, std::string_view>::type... columns)
    {
        if (params_.headers == false && headers_.empty()) {
            init_headers();
        }
        return Named<sizeof...(Args), std::tuple<Args...>>{*this, std::array{std::string_view{columns}...}};
    }

    /// Read the next record into the given parameters.
    ///
    /// The number of fields must match the number of parameters (or a
    /// `Error` is raised).
    ///
    /// Each field is converted to the corresponding type using `convert`. A
    /// `Error` is raised if the conversion fails.
    ///
    /// The function returns `true` if and only if a new record has been
    /// returned.
    template <typename... Args>
    bool get(Args&... args)
    {
        return get_impl({}, std::make_index_sequence<sizeof...(Args)>{}, args...);
    }

    /// Read the next record into a tuple.
    ///
    /// The number of fields must match the tuple size (or a `Error` is
    /// raised).
    ///
    /// Each field is converted to the corresponding type using `convert`. A
    /// `Error` is raised if the conversion fails.
    ///
    /// The function returns `true` if and only if a new record has been
    /// returned.
    template <typename... Args>
    bool get(std::tuple<Args...>& x)
    {
        return get_impl({}, x, std::make_index_sequence<sizeof...(Args)>{});
    }

    /// Read the next record into an array.
    ///
    /// The number of fields must match the array size (or a `Error` is
    /// raised).
    ///
    /// Each field is converted to the corresponding type using `convert`. A
    /// `Error` is raised if the conversion fails.
    ///
    /// The function returns `true` if and only if a new record has been
    /// returned.
    template <typename T, std::size_t N>
    bool get(std::array<T, N>& x)
    {
        return get_impl({}, x, std::make_index_sequence<N>{});
    }

    /// Read the next record into a vector.
    ///
    /// The vector will be resized to the number of fields.
    ///
    /// Each field is converted to the corresponding type using `convert`. A
    /// `Error` is raised if the conversion fails.
    ///
    /// The function returns `true` if and only if a new record has been
    /// returned.
    template <typename T>
    bool get(std::vector<T>& x);

    /// Read the next record into the given parameter.
    ///
    /// This function just calls `get`.
    template <typename Record>
    CsvReader& operator>>(Record& x)
    {
        get(x);
        return *this;
    }

    /// Return `true` if and only if there is another record to be read.
    explicit operator bool() const { return has_record_; }

    /// Return a `CsvReader` iterating over the records of the given input stream.
    static CsvReader read(std::istream& in, Parameters params = {}) { return CsvReader{in, params}; }

    /// Return a `CsvReader` iterating over the records of the given file.
    static CsvReader read(const std::string& filename, Parameters params = {});

private:
    /// Read the first line as header line.
    void init_headers();

    /// Read the next record.
    ///
    /// This method updates the internal data like `raw_` and `field_pos_`.
    ///
    /// Returns `true` on success.
    bool next_record();

    /// Remove whitespace at the end of the current field in `raw_`.
    void trim_whitespaces();

    template <typename... Args, std::size_t... I, std::size_t N = 0>
    bool get_impl(const std::array<std::size_t, N>& inds, std::index_sequence<I...>, Args&... x);

    template <typename... Args, std::size_t... I, std::size_t N = 0>
    bool get_impl(const std::array<std::size_t, N>& inds, std::tuple<Args...>& x, std::index_sequence<I...>)
    {
        return get_impl(inds, std::make_index_sequence<sizeof...(Args)>{}, std::get<I>(x)...);
    }

    template <typename T, std::size_t M, std::size_t... I, std::size_t N = 0>
    bool get_impl(const std::array<std::size_t, N>& inds, std::array<T, M>& x, std::index_sequence<I...>)
    {
        return get_impl(inds, std::make_index_sequence<M>{}, std::get<I>(x)...);
    }

    template <typename... Args, std::size_t N = 0>
    bool get_tuple(const std::array<std::size_t, N>& inds, std::tuple<Args...>& x)
    {
        return get_impl(inds, x, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <typename T, std::size_t M, std::size_t N = 0>
    bool get_tuple(const std::array<std::size_t, N>& inds, std::array<T, M>& x)
    {
        return get_impl(inds, x, std::make_index_sequence<M>{});
    }

    template <typename T>
    bool get_tuple(const std::array<std::size_t, 0>&, std::vector<T>& x)
    {
        return get(x);
    }

private:
    std::unique_ptr<std::istream> owned_in_ = nullptr;
    std::istream* in_;

    /// `true` if the last record has been read successfully.
    ///
    /// Used for the stream interface.
    bool has_record_ = true;

    /// The used read parameter.
    Parameters params_ = {};

    /// maximal number of columns
    std::size_t max_columns_ = 0;
    /// minimal number of columns
    std::size_t min_columns_ = std::numeric_limits<std::size_t>::max();
    /// current line number
    std::size_t line_ = 0;
    /// current record number
    std::size_t num_records_ = 0;

    /// The column headers.
    std::vector<std::string> headers_ = {};

    /// The raw data containing the current line's data.
    std::vector<char> raw_;

    /// Boundaries of fields in the current record (points to raw_)
    std::vector<std::pair<std::string::size_type, std::string::size_type>> field_pos_;
};

template <typename... Args, size_t... I, size_t N>
bool CsvReader::get_impl(const std::array<std::size_t, N>& inds, std::index_sequence<I...>, Args&... x)
{
    static_assert(N == 0 || N == sizeof...(Args), "Indices must match number of values");
    static constexpr auto size = N == 0 ? sizeof...(Args) : N;

    if (!next_record()) {
        has_record_ = false;
        return false;
    }

    if (N == 0 && (size != field_pos_.size())) {
        throw Error("Record has wrong number of fields (expected: " + std::to_string(size) +
                        " got:" + std::to_string(field_pos_.size()) + ")",
                    line_);
    }

    try {
        std::tie(x...) = std::make_tuple(convert<Args>(
            std::string_view{raw_.data() + field_pos_.at(N == 0 ? I : inds[I]).first,
                             field_pos_.at(N == 0 ? I : inds[I]).second - field_pos_.at(N == 0 ? I : inds[I]).first})...);
    } catch (std::invalid_argument& e) {
        has_record_ = false;
        throw Error("Error reading a field (" + std::string(e.what()) + ")", line_);
    } catch (std::out_of_range& e) {
        has_record_ = false;
        throw Error("Error reading a field (" + std::string(e.what()) + ")", line_);
    }

    has_record_ = true;

    return true;
}

template <typename T>
bool CsvReader::get(std::vector<T>& x)
{
    if (!next_record()) {
        has_record_ = false;
        return false;
    }

    x.clear();
    x.reserve(field_pos_.size());
    try {
        for (auto [b, e] : field_pos_) {
            x.emplace_back(convert<T>(std::string_view{raw_.data() + b, e - b}));
        }
    } catch (std::invalid_argument& e) {
        has_record_ = false;
        throw Error("Error reading a field (" + std::string(e.what()) + ")", line_);
    } catch (std::out_of_range& e) {
        has_record_ = false;
        throw Error("Error reading a field (" + std::string(e.what()) + ")", line_);
    }
    has_record_ = true;

    return true;
}

template <std::size_t N, typename Fields>
void CsvReader::Named<N, Fields>::compute_column_indices(const std::array<std::string_view, NumColumns>& columns)
{
    for (auto i : indices(columns)) {
        auto index = csv_.column_index(columns[i]);
        if (!index) {
            throw Error("Column '" + std::string(columns[i]) + "' not found in csv file");
        }
        indices_[i] = *index;
    }
}

extern template bool CsvReader::get(std::vector<std::string>& x);
extern template bool CsvReader::get(std::vector<std::string_view>& x);

}  // namespace util
}  // namespace fifr

#endif
