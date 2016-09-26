#include "mediaprocessor.h"

#include "ffmpegerror.h"

#include "framesprocessor.h"

Peaks MediaExtractor::ExtractPeaksAndSceneChanges()
{
    int ret;
    int sample_rate;
    int samples_per_peak;

    AVCodecContext *AudioCodecCtx = AudioStream->codec;
    AVCodecContext *VideoCodecCtx = nullptr;

    AVCodec *AudioCodec = avcodec_find_decoder(AudioCodecCtx->codec_id);
    //AVCodec *VideoCodec = avcodec_find_decoder(VideoCodecCtx->codec_id);

    if(!AudioCodec)
        throw FFmpegError("Could not find any decoder for this audio stream");
    //if(!VideoCodec)
     //   throw FFmpegError("Could not find any decoder for this video stream");

    ret = avcodec_open2(AudioCodecCtx, AudioCodec, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

    //ret = avcodec_open2(VideoCodecCtx, VideoCodec, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

    sample_rate = AudioCodecCtx->sample_rate;
    samples_per_peak = sample_rate / 100;

    Peaks PeakList(samples_per_peak, sample_rate);

    FramesProcessor Proc;
    connect(&Proc, SIGNAL(progress(int)), this, SLOT(trackProgress(int)));

    switch(AudioCodecCtx->sample_fmt)
    {
    case AV_SAMPLE_FMT_DBL:
        Proc.processFrames<double, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_DBLP:
        Proc.processFrames<double, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_FLT:
        Proc.processFrames<float, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_FLTP:
        Proc.processFrames<float, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S32:
        Proc.processFrames<int32_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S32P:
        Proc.processFrames<int32_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S16:
        Proc.processFrames<int16_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S16P:
        Proc.processFrames<int16_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_U8:
        Proc.processFrames<uint8_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_U8P:
        Proc.processFrames<uint8_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    default:
        throw FFmpegError("Sample format not supported");
    }

    avcodec_close(AudioCodecCtx);
    //avcodec_close(VideoCodecCtx);
    //emit finished();
    return std::move(PeakList);
}
