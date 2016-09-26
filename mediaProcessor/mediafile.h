#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <iterator>

extern "C"
{
#include <libavformat/avformat.h>
}

struct AVStream;

// FFmpeg type forward declaration
struct AVFormatContext;
struct AVPacket;

// This is the type of Files you want to read
// This type is not copyable, only moveable
class MediaFile
{
public:
    // Open a file for reading data
    // This function throws an FFmpegError if an error occurs
    MediaFile(const char *filename);
    MediaFile(const MediaFile&) = delete; // Disable copy constructor

    // Copy the pointer to the other FormatCtx
    // and set the other FormatCtx to nullptr
    // so that when other is deleted the file is not closed
    MediaFile(MediaFile &&other) :
        FormatCtx(other.FormatCtx)
    {
        other.FormatCtx = nullptr;
    }

    ~MediaFile();

    MediaFile &operator=(const MediaFile &) = delete; // Disable assignment

    AVStream **streams_begin();

    AVStream **streams_end();

    // Return the best stream of `type` type
    // If none is found return streams_end();
    AVStream **best_stream_of_type(AVMediaType type);

    AVDictionary *metadata() const
    {
        return FormatCtx->metadata;
    }

    double duration_in_seconds() const;

    // This function throws an FFmpegError if an error occurs
    int readNextPacket(AVPacket &pkt);

    // This function returns true if the packet was read
    // false otherwise
    bool getNextPacket(AVPacket &pkt) noexcept;
private:

    static void initAllAV();

    static bool avInitiated;

    AVFormatContext *FormatCtx;
};


#endif // MEDIAFILE_H
