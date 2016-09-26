#include "ffmpegerror.h"

extern "C"
{
#include <libavutil/avutil.h>
}

FFmpegError::FFmpegError(int error)
{
    char buffer[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string( buffer, AV_ERROR_MAX_STRING_SIZE, error);
    ErrorMessage = std::string(buffer);
}

FFmpegError::~FFmpegError()
{

}

const char *FFmpegError::what() const noexcept
{
    return ErrorMessage.c_str();
}

