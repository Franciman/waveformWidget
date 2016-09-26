#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include "waveformview.h"

#include <QGraphicsRectItem>
#include <QOpenGLWidget>

#include <iostream>

#include "srtParser/srtparser.h"

#include "renderer.h"

#include <QVideoWidget>

#include "mediaProcessor/mediafile.h"
#include "mediaProcessor/mediaprocessor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MediaFile file("/home/francesco/Desktop/vid.mp4");
    AVStream **AudioStream = file.best_stream_of_type(AVMEDIA_TYPE_AUDIO);
    Peaks P;
    if(AudioStream != file.streams_end())
    {
        MediaExtractor extractor(file, *AudioStream, nullptr, P);
        P = extractor.ExtractPeaksAndSceneChanges();
    }
    else
    {
        return;
    }

    QFile f("/home/francesco/Desktop/VO.srt");
    f.open(QFile::ReadOnly);
    SrtParser parser(&f);
    std::vector<SrtSubtitle> Subs = parser.parseSubs();
    auto Subs2 = Subs;
    f.close();

    RangeList *VO = new RangeList(std::move(Subs2), false);
    SubtitleData sdata(RangeList(std::move(Subs), true), VO);

    AbstractRenderer *R = new Renderer;
    R->loadMedia("/home/francesco/Desktop/vid.mp4");

    Waveform = new WaveformView(R, std::move(P), std::move(sdata), this);
    Waveform->setFixedHeight(300);
    setCentralWidget(Waveform);
}

MainWindow::~MainWindow()
{
    delete ui;
}
