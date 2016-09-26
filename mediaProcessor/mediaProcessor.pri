HEADERS += \
    $$PWD/mediaprocessor.h \
    $$PWD/mediafile.h \
    $$PWD/ffmpegerror.h \
    $$PWD/peaks.h \
    $$PWD/audio_frame_traits.h \
    $$PWD/sample_format_traits.h \
    $$PWD/sampleextractor.h \
    $$PWD/scenechangeextractor.h \
    $$PWD/framesprocessor.h

SOURCES += \
    $$PWD/mediaprocessor.cpp \
    $$PWD/mediafile.cpp \
    $$PWD/ffmpegerror.cpp \
    $$PWD/scenechangeextractor.cpp

PKGCONFIG += libavformat libavcodec libavutil libavfilter
