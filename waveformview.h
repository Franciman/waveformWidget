#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QAbstractScrollArea>
#include <QCache>
#include <QStaticText>

struct Peak
{
    int Min;
    int Max;
};

class WaveformViewport : public QWidget
{
    Q_OBJECT

    std::vector<Peak> Peaks;
    int SampleRate;
    int SamplesPerPeak;
    QImage OffscreenWav;

    int PageSizeMs;
    int PositionMs;

    int LengthMs;

    int OldPositionMs;
    int OldPageSizeMs;

    int VerticalScaling;

    int DisplayRulerHeight;

    QCache<QString, QStaticText> TimeStamps;

public:
    WaveformViewport(std::vector<Peak> &&peaks, int sampleRate, int samplesPerPeak, QWidget *parent = 0);

    void scrollContentsBy(int NewPositionMs);

    int length() const { return LengthMs; }
protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void resizeEvent(QResizeEvent *) override;
private:
    void paintWav(QPainter &painter, bool TryOptimize);
    void paintRuler(QPainter &painter);

    int timeToPixel(int time) const;
    int pixelToTime(int pixel) const;


};

class WaveformView : public QAbstractScrollArea
{
    Q_OBJECT

    WaveformViewport *Viewport;
public:
    WaveformView(std::vector<Peak> &&peaks, int sampleRate, int samplesPerPeak, QWidget *parent = 0);
protected:
    virtual void scrollContentsBy(int, int) override;
};

#endif // WAVEFORMVIEW_H
