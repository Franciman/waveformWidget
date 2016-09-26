#ifndef FFMPEG_ERROR_H
#define FFMPEG_ERROR_H

#include <string>
#include <exception>

class FFmpegError : public std::exception
{
public:
    FFmpegError(int error);
    FFmpegError(const std::string &errorMessage) :
        ErrorMessage(errorMessage)
    { }

    virtual ~FFmpegError();

    virtual const char *what() const noexcept override;

private:
    std::string ErrorMessage;
};

#endif // FFMPEG_ERROR_H
