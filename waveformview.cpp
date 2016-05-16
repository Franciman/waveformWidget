#include "waveformview.h"

#include <QResizeEvent>
#include <QPainter>
#include <QScrollBar>
#include <QHBoxLayout>
#include <cmath>

#include "timemstoshortstring.h"


const QColor WavColor = QColor(0x00a7f24a);
const QColor WavBackColor = QColor(Qt::black);
const QColor ZeroLineColor = QColor(0x00518b0a);
const QColor RangeColor1 = QColor(0x003333ff);
const QColor RangeColor2 = QColor(0x00ff8000);
const QColor RangeColorNotEditable = QColor(0x00c0c0c0);
const QColor RulerBackColor = QColor(0x00514741);
const QColor RulerTopBottomLineColor = QColor(0x00beb5ae);
const QColor RulerTextColor = QColor(0x00e0e0e0);
const QColor RulerTextShadowColor = QColor(Qt::black);
const QColor CursorColor = QColor(Qt::yellow);

WaveformViewport::WaveformViewport(std::vector<Peak> &&peaks, int sampleRate, int samplesPerPeak, QWidget *parent) :
    QWidget(parent),
    Peaks(std::move(peaks)),
    SampleRate(sampleRate),
    SamplesPerPeak(samplesPerPeak),
    OffscreenWav(size(), QImage::Format_ARGB32)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    OldPositionMs = PositionMs = 0;
    OldPageSizeMs = PageSizeMs = 8000;
    LengthMs = ((Peaks.size() * SamplesPerPeak) / SampleRate) * 1000;
    VerticalScaling = 100;
    DisplayRulerHeight = 20;
    QPainter imagePainter(&OffscreenWav);
    paintWav(imagePainter, false);
}


void WaveformViewport::paintWav(QPainter &painter, bool TryOptimize)
{
    //return;
    if(Peaks.empty())
    {
        painter.fillRect(rect(), WavBackColor);
        return;
    }

    int Width = timeToPixel(PageSizeMs);
    QRect Rect = rect();
    //double PeaksPerPixelScaled = double(TimePerPixel * SampleRateMs) / PeakList.samplesPerPeak();
    //double StartPositionInPeaks = double(PositionMs * SampleRateMs) / PeakList.samplesPerPeak();
    double PeaksPerPixelScaled = (((PageSizeMs / 1000.0) * SampleRate) / SamplesPerPeak) / Width;
    double StartPositionInPeaks =((PositionMs / 1000.0) * SampleRate) / SamplesPerPeak;

    int x1_update = 0;
    int x2_update = Width;

    if(TryOptimize && OldPageSizeMs == PageSizeMs)
    {
        int x_optim;
        if(PositionMs > OldPositionMs)
        {
           x_optim = timeToPixel(OldPositionMs - PositionMs);
           x2_update = x2_update - x_optim;
           x1_update = Width;
           if(x1_update != 0)
           {
              painter.drawImage(0, 0, OffscreenWav, x_optim, 0, x1_update, OffscreenWav.height() - DisplayRulerHeight);
           }
        }
        else
        {
            x_optim = timeToPixel(OldPositionMs - PositionMs);
            x1_update = 0;
            x2_update = x_optim;
            if(x2_update != Width)
            {
                painter.drawImage(x_optim, 0, OffscreenWav, 0, 0, Width - x_optim, OffscreenWav.height() - DisplayRulerHeight);
            }
        }
    }

    Rect.setLeft(x1_update);
    Rect.setRight(x2_update);
    Rect.setBottom(Rect.bottom() - DisplayRulerHeight);

    int RectHeight = Rect.height();
    int Middle = Rect.top() + (RectHeight / 2);

    painter.fillRect(Rect, WavBackColor);

    painter.setPen(WavColor);

    unsigned int x_scaled, next_x_scaled;
    int PeakMax, PeakMin;


    for(int x = x1_update; x <= x2_update; x++)
    {
        x_scaled = std::round((x * PeaksPerPixelScaled) + StartPositionInPeaks);

        if(x_scaled >= Peaks.size()) x_scaled = Peaks.size() - 1;

        PeakMax = Peaks[x_scaled].Max;
        PeakMin = Peaks[x_scaled].Min;

        next_x_scaled = std::min(std::round(((x + 1) * PeaksPerPixelScaled) + StartPositionInPeaks), (double)Peaks.size());
        for(unsigned int i = x_scaled + 1; i < next_x_scaled; i++)
        {
            if(Peaks[i].Max > PeakMax) PeakMax = Peaks[i].Max;
            else if(Peaks[i].Min < PeakMin) PeakMin = Peaks[i].Min;
        }

        int y1 = std::round((((PeakMax * VerticalScaling) / 100) * RectHeight) / 65536);
        int y2 = std::round((((PeakMin * VerticalScaling) / 100) * RectHeight) / 65536);
        painter.drawLine(QPoint(x, Middle - y1), QPoint(x, Middle - y2));
    }
}

void WaveformViewport::paintRuler(QPainter &painter)
{
    //return;
    if(DisplayRulerHeight > 0)
    {
        QRect Rect = rect();
        Rect.setTop(Rect.bottom() - DisplayRulerHeight);

        // Draw background
        painter.fillRect(Rect,RulerBackColor);

        // Draw horizontal line at top and bottom
        painter.setPen(RulerTopBottomLineColor);
        painter.drawLine(QPoint(0, Rect.top()), QPoint(width(), Rect.top()));
        painter.drawLine(QPoint(0, Rect.bottom() - 1), QPoint(width(), Rect.bottom() - 1));

        // Set the text font
        QFont font("Time New Roman", 8);
        painter.setFont(font);
        painter.setPen(RulerTextColor);
        QFontMetrics metrics(font);

        // Do some little calculation to try to show "round" time
        int MaxPosStep = std::round(width() / (metrics.width("0:00:00.0") * 2));
        int StepMs = std::round(PageSizeMs / MaxPosStep);
        if(StepMs == 0) StepMs = 1;

        int StepLog = std::trunc(std::pow(10, std::trunc(std::log10(StepMs))));
        StepMs = (StepMs / StepLog) * StepLog;

        int p = PositionMs / (StepMs * StepMs);
        int x;
        int x1, x2;
        //int height, width;
        QString PosString;
        while(p < PositionMs + PageSizeMs)
        {
            // Draw main division
            x = timeToPixel(p - PositionMs);
            painter.drawLine(QPoint(x, Rect.top() + 1), QPoint(x, Rect.top() + 5));

            PosString = timeMsToShortString(p, StepLog);

            // Calculate text coordinates
            x1 = x - metrics.width(PosString) / 2;
            //height = metrics.height();
            //width = metrics.width(PosString);

            if(TimeStamps.contains(PosString))
            {
                // Draw text shadow
                painter.setPen(RulerTextShadowColor);
                painter.drawStaticText(x1 + 2, Rect.top() + 4, *TimeStamps.object(PosString));
                painter.setPen(RulerTextColor);
                painter.drawStaticText(x1, Rect.top() + 4, *TimeStamps.object(PosString));
            }
            else
            {
                QStaticText *timestamp = new QStaticText(PosString);
                TimeStamps.insert(PosString, timestamp);
                // Draw text shadow
                painter.setPen(RulerTextShadowColor);
                painter.drawStaticText(x1 + 2, Rect.top() + 4, *timestamp);

                // Draw text
                painter.setPen(RulerTextColor);
                painter.drawStaticText(x1, Rect.top() + 4, *timestamp);
            }
            // Draw subdivision
            x2 = x + timeToPixel(StepMs / 2);
            painter.drawLine(QPoint(x2, Rect.top() + 1), QPoint(x2, Rect.top() + 3));
            p += StepMs;
        }
    }
}

void WaveformViewport::scrollContentsBy(int NewPositionMs)
{
    if(NewPositionMs + PageSizeMs - 1 > LengthMs)
    {
        PositionMs = LengthMs - PageSizeMs;
    }
    else
    {
        PositionMs = NewPositionMs;
    }
    update();
}

void WaveformViewport::paintEvent(QPaintEvent *)
{
    {
        QPainter imagePainter(&OffscreenWav);
        paintWav(imagePainter, false);
    }
    QPainter painter(this);
    painter.drawImage(0, 0, OffscreenWav);
    paintRuler(painter);
}

void WaveformViewport::resizeEvent(QResizeEvent *)
{
    OffscreenWav = QImage(size(), QImage::Format_ARGB32);
    QPainter imagePainter(&OffscreenWav);
    paintWav(imagePainter, false);
}

int WaveformViewport::timeToPixel(int time) const
{
    if(PageSizeMs == 0)
        return 0;
    else
        return std::round(time / (PageSizeMs / width()));
}

int WaveformViewport::pixelToTime(int pixel) const
{
    return std::round(pixel * (PageSizeMs / width()));
}

WaveformView::WaveformView(std::vector<Peak> &&peaks, int sampleRate, int samplesPerPeak, QWidget *parent) :
        QAbstractScrollArea(parent),
        Viewport(new WaveformViewport(std::move(peaks), sampleRate, samplesPerPeak, this))
{
    Viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(Viewport);
    QWidget *scrollWidget = new QWidget(this);
    scrollWidget->setLayout(layout);
    setViewport(scrollWidget);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    horizontalScrollBar()->setRange(0, Viewport->length());
    horizontalScrollBar()->setSingleStep(50);
    horizontalScrollBar()->setValue(0);
}

void WaveformView::scrollContentsBy(int, int)
{
    Viewport->scrollContentsBy(horizontalScrollBar()->value());
}
