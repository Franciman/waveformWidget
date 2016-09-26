#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QOpenGLWidget>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>

#include <cmath>

#include <QPainter>
#include <QTimer>

#include "mediaProcessor/peaks.h"
#include "constrain.h"

#include "model.h"

#include <iostream>

#include <algorithm>

class AbstractRenderer;

class WaveformViewport : public QOpenGLWidget
{
    Q_OBJECT

    // Model data ----
    Peaks PData;
    SubtitleData SData;
    // ---------------

    AbstractRenderer *Rend;

    std::vector<RangeList *> DisplayRangeLists;

    enum FocusingMode
    {
        FocusBegin,
        FocusEnd,
        FocusNone
    };



public:
    WaveformViewport(AbstractRenderer *rend, Peaks &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr);

    // Getters and setters --------------------
    void setPosition(int position)
    {
        PositionMs = position;
        //update();
    }

    void incrementPosition(int increment)
    {
        PositionMs += increment;
    }

    int position() const
    {
        return PositionMs;
    }

    int audioLength() const
    {
        return (PData.peaksNumber() * PData.samplesPerPeak()) / PData.sampleRate();
    }

    void setPageSize(int pageSize)
    {
        PageSizeMs = pageSize;
    }

    int pageSize() const
    {
        return PageSizeMs;
    }
    // --------------------------------------


protected:
    void paintGL() override
    {
        QPixmap offscreen(size());
        QPainter painter(&offscreen);
        paintWav(painter);
        paintRuler(painter);
        paintMinimumBlank(painter, 0, offscreen.height() - 1);
        paintRangeLists(painter);
        paintSelection(painter);
        paintCursor(painter);
        paintPlayCursor(painter);
        QPainter p2(this);
        p2.drawPixmap(0, 0, offscreen);
    }

    void wheelEvent(QWheelEvent *ev) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    // Time to Pixel and viceversa conversion ---
    inline unsigned int timeToPixel(int Time) const
    {
        double PixelPerMs = width() / double(PageSizeMs);
        return std::round(PixelPerMs * Time);
    }

    inline unsigned int relTimeToPixel(int Time) const
    {
        return timeToPixel(Time - PositionMs);
    }

    inline unsigned int pixelToRelTime(int Pixel) const
    {
        double MsPerPixel = double(PageSizeMs) / width();
        return std::round(MsPerPixel * Pixel);
    }

    inline unsigned int pixelToTime(int Pixel) const
    {
        return pixelToRelTime(Pixel) + PositionMs;
    }

    // ------------------------------------------


    void paintWav(QPainter &painter);
    void paintRuler(QPainter &painter);
    void paintRangeLists(QPainter &painter);
    void paintRanges(QPainter &painter, RangeList &Subs, int topPos, int bottomPos, bool topLine, bool bottomLine);
    void paintSelection(QPainter &painter);
    void paintCursor(QPainter &painter);
    void paintPlayCursor(QPainter &painter);
    void paintMinimumBlank(QPainter &painter, int rangeTop, int rangeBottom);

    // Utilities ----------------------------------------

    bool isPositionVisible(int PosMs) const
    {
        return PositionMs <= PosMs && PosMs <= PositionMs + PageSizeMs;
    }

    RangeList &getRangeListFromPos(int y)
    {
        int SubHeight = (height() - RulerHeight) / DisplayRangeLists.size();
        int index = std::floor(y / SubHeight);
        Constrain(index, 0, (int)DisplayRangeLists.size() - 1);
        return *DisplayRangeLists[index];
    }

    bool hasFocusedSubtitle()
    {
        return FocusedSubtitle != SData.end();
    }

    bool hasSelection()
    {
        return Selection.StartTime >= 0;
    }

    bool findFocusingSubtitle(int CursorPosMs, RangeList &RL, int ToleranceFromBorder, int PositionTolerance);

    static bool checkRangeForFocusing(Range &R, int CursorPosMs, int ToleranceFromBorder, FocusingMode &NewMode, int &NewFocusTime);

    int findSnappingPoint(int PosMs, RangeList &RL);
    int findCorrectedSnappingPoint(int PosMs, RangeList &RL);

    // ----------------------------------------------------

    void mousePressCoolEdit(QMouseEvent *ev, RangeList &RangeListClicked);

private:
    int PositionMs = 0; // This indicates the portion of waveform to draw
    int PageSizeMs = 15000; // This represents horizontal zoom
    int VerticalScaling = 100; // This represents vertical zoom

    int CursorMs = 100; // Cursor position in milliseconds

    bool IsPlaying = true; // Flag indicating whether the media is being played
    int PlayCursorMs = 200; // Play Cursor position in milliseconds

    int RulerHeight = 0; // Ruler height, if 0, the ruler won't be displayed

    Range Selection = Range { -1, 0 }; // Range representing selection, if the selection is present Selection.StartTime >= 0 otherwise it's < 0

    int SelectionOriginMs = -1; // The point where the mouse was clicked before a selection was highlighted, if valid its value is >= 0, otherwise it's < 0


    FocusingMode FocusMode = FocusNone; // Flag indicating if the Focused time is left or right limit of a range, if set to FocusNone, no time is focused
    int FocusedTimeMs; // Focused limit of a range
    SubtitleData::iterator FocusedSubtitle; // If the focused time is of a subtitle, this iterator points to the desired subtitle, otherwise it's end() of SubtitleData
    SubtitleData::iterator OldFocusedSubtitle; // Previous focused subtitle

    bool MouseDown = false; // Flag indicating whether the left mouse button is currently down

    bool EnableSnapping = true; // Flag indicating whether to adjust mouse pos to a snapping point

    bool EnableMouseAntiOverlapping = true; // Flag indicating whether to prevent subtitle overlapping

    // Anti overlapping informations
    int MinSelTime = -1; // Minimum valid time, if valid it's >= 0
    int MaxSelTime = -1; // Maximum valid time, if valid it's >= 0

    bool SubChanged = false; // Flag indicating whether to sort subs

    // int UpdateIntervalMs = 17; // Set to 17 ms in order to try to achieve 60 fps

    // QTimer PlayCursorUpdater;

    bool ShowMinBlank = true;
    int MinimumBlankMs = 1; //Minimum blank between subtitles
    MinBlankInfo Info1;
    MinBlankInfo Info2;

private slots:
    void updatePlayCursorPos();
    void updatePlayCursorPos(int PosMs);
};

class WaveformView : public QAbstractScrollArea
{
public:
    WaveformView(AbstractRenderer *R, Peaks &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr) :
        QAbstractScrollArea(parent),
        Viewport(new WaveformViewport(R, std::move(pdata), std::move(sdata), this))
    {
        Viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(Viewport);
        viewport()->setLayout(layout);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        horizontalScrollBar()->setRange(0, Viewport->audioLength() * 1000);
        horizontalScrollBar()->setSingleStep(50);
    }

protected:
    void scrollContentsBy(int, int) override
    {
        Viewport->setPosition(horizontalScrollBar()->value());
        Viewport->update();
    }

private:
    WaveformViewport *Viewport;
};

#endif // WAVEFORMVIEW_H
