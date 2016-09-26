#ifndef FRAMESPROCESSOR_H
#define FRAMESPROCESSOR_H

#include "peaks.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "mediafile.h"
#include "sampleextractor.h"
#include "scenechangeextractor.h"

#include <functional>

#include <QObject>


struct FramesProcessor : public QObject
{
    Q_OBJECT
private:
    void trackProgress(int total, int val)
    {
        emit progress((val * 100) / total);
    }

public:
    template<class SampleFormat, bool Planar>
    void processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList);

Q_SIGNALS:
    void progress(int);
};

template<class SampleFormat, bool Planar>
void FramesProcessor::processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList)
{
   AVPacket pkt;
   AVFrame *frame = av_frame_alloc();

   if(!frame)
       throw FFmpegError("Could not allocate memory for frame");

   av_init_packet(&pkt);
   pkt.data = nullptr;
   pkt.size = 0;

   int audio_stream_index = audioStream->index;
   int video_stream_index = videoStream ? videoStream->index : -1;

   using namespace std::placeholders;

   PeaksExtractor<SampleFormat, Planar> PExtractor(std::bind(&FramesProcessor::trackProgress, this, media.duration_in_seconds(), _1), AudioCodecCtx, PeakList);
   //SceneChangeExtractor SCExtractor(videoStream, VideoCodecCtx, SceneChanges);

   while(media.getNextPacket(pkt))
   {
      AVPacket orig_pkt = pkt;
      if(pkt.stream_index == audio_stream_index)
      {
          PExtractor(pkt);
      }
      else if(pkt.stream_index == video_stream_index)
      {
          //SCExtractor(pkt);
      }
      av_packet_unref(&orig_pkt);
   }

   PExtractor.flush();
   //SCExtractor.flush();
   PExtractor.normalizePeaks();
}


#endif // FRAMESPROCESSOR_H
