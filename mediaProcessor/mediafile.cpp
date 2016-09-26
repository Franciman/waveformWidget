#include "mediafile.h"

#include "ffmpegerror.h"

extern "C"
{
#include <libavfilter/avfilter.h>
}

bool MediaFile::avInitiated = false;

MediaFile::MediaFile(const char *filename) :
    FormatCtx(nullptr)
{
    // If not registered, register formats and codecs
    initAllAV();

    // Open the input file
    int result = avformat_open_input(&FormatCtx, filename, nullptr, nullptr);
    if(result < 0)
    {
        // Issue an error
        throw FFmpegError(result);
    }

    //Retrieve stream informations
    result = avformat_find_stream_info(FormatCtx, nullptr);
    if(result < 0)
    {
        // Issue an error
        throw FFmpegError(result);
    }
}

MediaFile::~MediaFile()
{
    avformat_close_input(&FormatCtx);
}

AVStream **MediaFile::streams_begin()
{
    return FormatCtx->streams;
}

AVStream **MediaFile::streams_end()
{
    return FormatCtx->streams + FormatCtx->nb_streams;
}

AVStream **MediaFile::best_stream_of_type(AVMediaType type)
{
    int result = av_find_best_stream(FormatCtx, type, -1, -1, nullptr, 0);
    if(result < 0)
    {
        // Issue an error
        //throw FFmpegError(result);
        return streams_end();
    }
    // result indicates the index of the stream we're looking for in FormatCtx->streams
    return FormatCtx->streams + result;
}

int MediaFile::readNextPacket(AVPacket &pkt)
{
    int res = av_read_frame(FormatCtx,  &pkt);
    if(res < 0)
    {
       // Issue an error
        throw FFmpegError(res);
    }
    return res;
}

bool MediaFile::getNextPacket(AVPacket &pkt) noexcept
{
    return av_read_frame(FormatCtx, &pkt) >= 0;
}

double MediaFile::duration_in_seconds() const
{
    return static_cast<double>(FormatCtx->duration) / AV_TIME_BASE;
}

void MediaFile::initAllAV()
{
    if(!avInitiated)
    {
        av_register_all();
        avfilter_register_all();
        avInitiated = true;
    }
}
