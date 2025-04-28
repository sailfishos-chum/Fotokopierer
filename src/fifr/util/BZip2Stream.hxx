#ifndef __FIFRUTIL_BZIP2STREAM_HXX__
#define __FIFRUTIL_BZIP2STREAM_HXX__

#include "ZipStreamBase.hxx"

#include <memory>
#include <streambuf>
#include <string>

namespace fifr {
namespace util {
/// Stream buffer for bzip2ed files.
class BZip2StreamBuf : public std::streambuf
{
public:
    /// Constructor.
    BZip2StreamBuf();

    BZip2StreamBuf(BZip2StreamBuf&&) = delete;
    BZip2StreamBuf(const BZip2StreamBuf&) = delete;
    BZip2StreamBuf& operator=(BZip2StreamBuf&&) = delete;
    BZip2StreamBuf& operator=(const BZip2StreamBuf&) = delete;

    /// Destructor.
    ///
    /// Closes the stream.
    ~BZip2StreamBuf() override;

    /// Returns true if the associated file is opened.
    [[nodiscard]] bool is_open() const;

    ///
    /// Opens a file.
    ///
    /// A gzipped file may neither be opened read-write (at the
    /// same time) nor in append-mode.
    ///
    /// \param fname The name of the file to open.
    /// \param op_mode The mode in which the file should be opened.
    ///
    /// \return \b this on success, `nullptr` otherwise.
    ///
    BZip2StreamBuf* open(const std::string& name, std::ios_base::openmode op_mode);

    /// Closes the associated file.
    BZip2StreamBuf* close();

    int overflow(int c = EOF) override;

    int underflow() override;

    int sync() override;

private:
    /// Flushes the current write buffe to the file.
    int flush_buffer();

private:
    struct Data;
    std::unique_ptr<Data> d;
};

using InputBZip2Stream = InputZipStreamBase<BZip2StreamBuf>;

using OutputBZip2Stream = OutputZipStreamBase<BZip2StreamBuf>;

using ibz2stream = InputBZip2Stream;

using obz2stream = OutputBZip2Stream;
}  // namespace util
}  // namespace fifr

#endif
