#ifndef AUDIO_FRAME_TRAITS_H
#define AUDIO_FRAME_TRAITS_H

extern "C"
{
#include <libavcodec/avcodec.h>
}

template<class SampleFormat, bool Planar>
struct audio_frame_traits
{
   template<class FunctionT>
    static void for_each_sample(AVFrame *Frame, FunctionT func);
};

template<class SampleFormat>
struct audio_frame_traits<SampleFormat, true>
{
    template<class FunctionT>
    static void for_each_sample(AVFrame *Frame, FunctionT func)
    {
        SampleFormat **Buffer = reinterpret_cast<SampleFormat**>(Frame->data);
        for(int j = 0; j < Frame->nb_samples; ++j)
        {
            for(int i = 0; i < Frame->channels; ++i)
            {
                func(Buffer[i][j]);
            }
        }
    }
};

template<class SampleFormat>
struct audio_frame_traits<SampleFormat, false>
{
    template<class FunctionT>
    static void for_each_sample(AVFrame *Frame, FunctionT func)
    {
        SampleFormat *Buffer = reinterpret_cast<SampleFormat *>(Frame->data[0]);
        for(int i = 0; i < Frame->nb_samples * Frame->channels; ++i)
        {
            func(Buffer[i]);
        }
    }
};


#endif // AUDIO_FRAME_TRAITS_H
