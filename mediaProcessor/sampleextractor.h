#ifndef SAMPLEEXTRACTOR_H
#define SAMPLEEXTRACTOR_H

#include "audio_frame_traits.h"
#include "sample_format_traits.h"

#include <memory>
#include <iostream>

#include "peaks.h"
#include "ffmpegerror.h"

#include <functional>

template<class SampleFormat, bool Planar>
class PeaksExtractor
{
    std::function<void(int)> Callback;
    AVCodecContext *CodecCtx;
    int SamplesPerPeak;
    int SamplesConsidered;
    bool GotFirstSample;
    AVFrame *Frame;
    Peaks &Result;
    Peak CurrPeak;

public:
    PeaksExtractor(std::function<void(int)> callback, AVCodecContext *codecCtx, Peaks &result) :
        Callback(callback),
        CodecCtx(codecCtx),
        SamplesConsidered(0),
        GotFirstSample(false),
        Result(result)
    {
        SamplesPerPeak = Result.samplesPerPeak();
        if(SamplesPerPeak <= 0)
        {
           throw FFmpegError("The number of samples per peak must be an integer greater than 0") ;
        }
        Frame = av_frame_alloc();
        if(!Frame)
        {
            throw FFmpegError("Could not allocate frame, out of memory");
        }
    }

    ~PeaksExtractor()
    {
        av_frame_free(&Frame);
    }

    void operator()(AVPacket &pkt)
    {
        int got_frame;
        do
        {
            int ret = avcodec_decode_audio4(CodecCtx, Frame, &got_frame, &pkt);
            if(ret < 0)
            {
                throw FFmpegError(ret);
            }
            if(got_frame)
            {
                processFrame();
            }
            pkt.size -= ret;
            pkt.data += ret;
        } while(pkt.size > 0);
    }

    void flush()
    {
        int got_frame;
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;
        while(true)
        {
            avcodec_decode_audio4(CodecCtx, Frame, &got_frame, &pkt);
            if(got_frame)
            {
                processFrame();
            }
            else
            {
                break;
            }
        }
        //av_free_packet(&pkt);

        // Push the remainder of samples
        if(SamplesConsidered != 0)
        {
            if(CurrPeak.min() < Result.minPeak()) Result.updateMinPeak(CurrPeak.min());
            else if(CurrPeak.max() > Result.maxPeak()) Result.updateMaxPeak(CurrPeak.max());
            Result.addPeak(CurrPeak);
        }
    }

private:
    void processFrame()
    {
        int timeStamp = av_frame_get_best_effort_timestamp(Frame);
        Callback(timeStamp);
        audio_frame_traits<SampleFormat, Planar>::for_each_sample(Frame, [this](SampleFormat sample) {
            this->processSample(sample);
        });
        return;
    }

    void processSample(SampleFormat sample)
    {
        int32_t ConvertedSample = sample_format_traits<SampleFormat>::convertToInt32(sample);
        if(SamplesConsidered == 0)
        {
            if(!GotFirstSample)
            {
                Result.updateMinPeak(ConvertedSample);
                Result.updateMaxPeak(ConvertedSample);
                GotFirstSample = true;
            }
            CurrPeak.min(ConvertedSample);
            CurrPeak.max(ConvertedSample);
            ++SamplesConsidered;
        }
        else
        {
            if(ConvertedSample < CurrPeak.min()) CurrPeak.min(ConvertedSample);
            else if(ConvertedSample > CurrPeak.max()) CurrPeak.max(ConvertedSample);
            ++SamplesConsidered;
        }

        if(SamplesConsidered == SamplesPerPeak * Frame->channels)
        {
            if(CurrPeak.min() < Result.minPeak()) Result.updateMinPeak(CurrPeak.min());
            if(CurrPeak.max() > Result.maxPeak()) Result.updateMaxPeak(CurrPeak.max());
            Result.addPeak(CurrPeak);
            SamplesConsidered = 0;
        }

    }

public:
    void normalizePeaks()
    {
        int maxAbsValue = std::max(std::abs(Result.minPeak()), std::abs(Result.maxPeak()));
        double normFactor = static_cast<double>(INT16_MAX + 1) / maxAbsValue;
        if(normFactor > 1.1)
        {
            for(auto p = Result.peaks_begin(); p != Result.peaks_end(); ++p)
            {
                p->min(std::round(p->min() * normFactor));
                p->max(std::round(p->max() * normFactor));
            }
        }
    }
};


#endif // SAMPLEEXTRACTOR_H
