#ifndef SCENECHANGEEXTRACTOR_H
#define SCENECHANGEEXTRACTOR_H

#include <QVector>

struct AVCodecContext;
struct AVFilterContext;
struct AVFilterGraph;
struct AVStream;
struct AVPacket;
struct AVFrame;

class SceneChangeExtractor
{
    AVCodecContext *CodecCtx;
    QVector<int> &Result;
    AVFilterContext *BufferSrcCtx;
    AVFilterContext *BufferSinkCtx;
    AVFilterGraph *FilterGraph;
    AVFrame *Frame;
    AVFrame *FilteredFrame;
    double TimeBase;
public:
    SceneChangeExtractor(AVStream *videoStream, AVCodecContext *codecCtx, QVector<int> &result);
    ~SceneChangeExtractor();

    void operator()(AVPacket &pkt);
    void flush();
private:
    void init_filters(AVStream *videoStream);
    void processFrame();

    int timeBaseToMs(int time)
    {
        return time * TimeBase * 1000;
    }
};

#endif // SCENECHANGEEXTRACTOR_H
