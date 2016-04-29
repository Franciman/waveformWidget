#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fstream>
#include "waveformview.h"

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
    Waveform = new WaveformView(std::move(Peaks), SampleRate, SamplesPerPeak, this);
    Waveform->setFixedHeight(300);
    setCentralWidget(Waveform);
}

MainWindow::~MainWindow()
{
    delete ui;
}
