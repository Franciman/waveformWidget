#include "scenechangeextractor.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavformat/avformat.h>
}

#include "ffmpegerror.h"
#include <cstdio>

#include <memory>

SceneChangeExtractor::SceneChangeExtractor(AVStream *videoStream, AVCodecContext *codecCtx, QVector<int> &result) :
    CodecCtx(codecCtx),
    Result(result),
    BufferSrcCtx(nullptr),
    BufferSinkCtx(nullptr),
    FilterGraph(nullptr),
    Frame(av_frame_alloc()),
    FilteredFrame(av_frame_alloc())
{
    if(!Frame || !FilteredFrame)
    {
        throw FFmpegError("Could not allocate frame, out of memory");
    }
    init_filters(videoStream);
}

void SceneChangeExtractor::init_filters(AVStream *videoStream)
{
    char args[512];
    AVFilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVRational time_base = videoStream->time_base;
    TimeBase = static_cast<double>(time_base.num) / time_base.den;

    const char *filter_descr="select=gt(scene\\,.3)";

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    if(!outputs || !inputs)
    {
        throw FFmpegError("Could not allocate memory for filter graph");
    }

    FilterGraph = avfilter_graph_alloc();
    if(!FilterGraph)
        throw FFmpegError("Could not allocate memory for filter graph");

    std::snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                  CodecCtx->width, CodecCtx->height, CodecCtx->pix_fmt, time_base.num, time_base.den,
                  CodecCtx->sample_aspect_ratio.den, CodecCtx->sample_aspect_ratio.den);

    int ret = avfilter_graph_create_filter(&BufferSrcCtx, buffersrc, "in", args, nullptr, FilterGraph);
    if(ret < 0)
        throw FFmpegError(ret);

    ret = avfilter_graph_create_filter(&BufferSinkCtx, buffersink, "out", nullptr, nullptr, FilterGraph);
    if(ret < 0)
        throw FFmpegError(ret);

    outputs->name = av_strdup("in");
    outputs->filter_ctx = BufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = BufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    ret = avfilter_graph_parse_ptr(FilterGraph, filter_descr, &inputs, &outputs, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

    ret = avfilter_graph_config(FilterGraph, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

}

SceneChangeExtractor::~SceneChangeExtractor()
{
    avfilter_graph_free(&FilterGraph);
    av_frame_free(&Frame);
    av_frame_free(&FilteredFrame);
}

void SceneChangeExtractor::operator ()(AVPacket &pkt)
{
    int got_frame;
    int ret = avcodec_decode_video2(CodecCtx, Frame, &got_frame, &pkt);
    if(ret < 0)
        throw FFmpegError(ret);

    if(got_frame)
    {
        processFrame();
    }
}

void SceneChangeExtractor::processFrame()
{
    int ret;
    Frame->pts = av_frame_get_best_effort_timestamp(Frame);
    ret = av_buffersrc_add_frame(BufferSrcCtx, Frame);
    if(ret < 0)
        throw FFmpegError(ret);

    while(1)
    {
        ret = av_buffersink_get_frame(BufferSinkCtx, FilteredFrame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if(ret < 0)
            throw FFmpegError(ret);

        Result.push_back(timeBaseToMs(FilteredFrame->pts));
        av_frame_unref(FilteredFrame);
    }
}

void SceneChangeExtractor::flush()
{
    int got_frame;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    while(true)
    {
        avcodec_decode_video2(CodecCtx, Frame, &got_frame, &pkt);
        if(got_frame)
        {
            processFrame();
        }
        else
        {
            break;
        }
    }
    av_free_packet(&pkt);
}
