#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include "waveformview.h"

#include <QGraphicsRectItem>
#include <QOpenGLWidget>

#include <iostream>

#include "srtParser/srtparser.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::ifstream input("/home/francesco/Desktop/PeakFile");
    int SampleRate, SamplesPerPeak;
    input >> SampleRate;
    input >> SamplesPerPeak;
    std::vector<Peak> Peaks;
    int Min, Max;
    while(input)
    {
        input >> Min;
        input >> Max;
        Peaks.push_back(Peak { Min, Max });
    }
    input.close();

    QFile f("/home/francesco/Desktop/VO.srt");
    f.open(QFile::ReadOnly);
    SrtParser parser(&f);
    std::vector<SrtSubtitle> Subs = parser.parseSubs();
    auto Subs2 = Subs;
    f.close();
    PeakData data = PeakData{ std::move(Peaks), SampleRate, SamplesPerPeak };

    RangeList *VO = new RangeList(std::move(Subs2), false);
    SubtitleData sdata(RangeList(std::move(Subs), true), VO);
    Waveform = new WaveformView(std::move(data), std::move(sdata), this);
    Waveform->setFixedHeight(300);
    setCentralWidget(Waveform);
}

MainWindow::~MainWindow()
{
    delete ui;
}
