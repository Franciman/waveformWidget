#ifndef MEDIAPROCESSOR_H
#define MEDIAPROCESSOR_H

#include <QVector>
#include <QThread>
#include "peaks.h"


#include <exception>

class MediaFile;

struct AVStream;

class MediaExtractor : public QThread
{
    Q_OBJECT

public:
    MediaExtractor(MediaFile &file, AVStream *audioStream, AVStream *videoStream, Peaks &peaks) :
        Media(file),
        AudioStream(audioStream),
        VideoStream(videoStream),
        P(peaks)
    { }

    virtual void run() override
    {
        try
        {
            P = ExtractPeaksAndSceneChanges();
        }
        catch(std::exception &err)
        {
            ExceptionPtr = std::current_exception();
        }
        emit finished();
    }

    const std::exception_ptr &getException() const
    {
        return ExceptionPtr;
    }

    Peaks ExtractPeaksAndSceneChanges();

Q_SIGNALS:
    void progress(int percent);
    void finished();

private Q_SLOTS:
    void trackProgress(int percent)
    {
        emit progress(percent);
    }

private:
    std::exception_ptr ExceptionPtr;
    MediaFile &Media;
    AVStream *AudioStream;
    AVStream *VideoStream;
    Peaks &P;
};

#endif // MEDIAPROCESSOR_H
