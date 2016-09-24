#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QOpenGLWidget>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>

#include <cmath>

#include <QPainter>
#include <QTimer>

#include "srtParser/srtsubtitle.h"
#include "constrain.h"

#include <iostream>

#include <algorithm>

class AbstractRenderer;

struct Peak
{
    int Min;
    int Max;
};

struct PeakData
{
    std::vector<Peak> Data;
    int SampleRate;
    int SamplesPerPeak;
};

class RangeList
{
    std::vector<SrtSubtitle> Subs;
    bool Editable;
public:
    typedef std::vector<SrtSubtitle>::iterator iterator;
    typedef std::vector<SrtSubtitle>::const_iterator const_iterator;
    typedef std::vector<SrtSubtitle>::size_type size_type;

    RangeList(std::vector<SrtSubtitle> &&subs, bool editable = true) :
        Subs(std::move(subs)),
        Editable(editable)
    {
        sortSubs();
    }

    iterator begin()
    {
        return Subs.begin();
    }

    const_iterator cbegin() const
    {
        return Subs.cbegin();
    }

    iterator end()
    {
        return Subs.end();
    }

    const_iterator cend() const
    {
        return Subs.cend();
    }

    const SrtSubtitle &operator[](size_type index) const
    {
        return Subs[index];
    }

    void addSubtitle(SrtSubtitle &&sub);

    iterator subsAheadOf(int StartPosMs)
    {
        return std::lower_bound(Subs.begin(), Subs.end(), StartPosMs, [](const SrtSubtitle &Sub, int PosMs) -> bool
        {
            return Sub.Time.EndTime < PosMs;
        });
    }

    bool editable() const
    {
        return Editable;
    }

    void setEditable(bool value)
    {
        Editable = value;
    }

    void sortSubs()
    {
        std::sort(Subs.begin(), Subs.end(), [](const SrtSubtitle &s1, const SrtSubtitle &s2)
        {
            if(s1.Time.StartTime < s2.Time.StartTime) return true;
            else if(s1.Time.StartTime == s2.Time.StartTime) return s1.Time.EndTime < s2.Time.EndTime;
            else return false;
        });
    }

    iterator getNearestSubtitleAt(int PosMs)
    {
        auto it = subsAheadOf(PosMs);
        auto result = Subs.end();
        while(it != Subs.end() && it->Time.StartTime <= PosMs)
        {
            result = it;
            ++it;
        }
        return result;
    }
};

class SubtitleData
{
public:
    typedef RangeList::iterator iterator;

private:
    // This list is always sorted based on time
    // So that it can be searched in log(n) time
    RangeList Subs;
    RangeList *VO;
    iterator Selected;
public:

    SubtitleData(RangeList &&subs, RangeList *vo = nullptr) :
        Subs(std::move(subs)),
        VO(vo),
        Selected(Subs.end())
    {
    }

    bool hasVO() const
    {
        return VO;
    }

    const SrtSubtitle &operator[](RangeList::size_type index) const
    {
        return Subs[index];
    }

    iterator begin()
    {
        return Subs.begin();
    }

    iterator end()
    {
        return Subs.end();
    }

    void addSubtitle(SrtSubtitle &&sub);

    RangeList *vo()
    {
        return VO;
    }

    RangeList *subs()
    {
        return &Subs;
    }

    void setSelectedSubtitle(iterator sub)
    {
        Selected = sub;
    }

    iterator selectedSubtitle()
    {
        return Selected;
    }

    bool hasSelected()
    {
        return Selected != Subs.end();
    }
};

class WaveformViewport : public QOpenGLWidget
{
    Q_OBJECT

    // Model data ----
    PeakData PData;
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

    FocusingMode FocMode = FocusNone;
    int FocusingTimeMs; // Only valid when FocMode != FocusNone
    SubtitleData::iterator FocusedSubtitle;
public:
    WaveformViewport(AbstractRenderer *rend, PeakData &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr) :
        QOpenGLWidget(parent),
        PData(std::move(pdata)),
        SData(std::move(sdata)),
        Rend(rend),
        FocusedSubtitle(SData.end())
    {
        if(SData.hasVO())
        {
            DisplayRangeLists.push_back(SData.vo());
        }
        DisplayRangeLists.push_back(SData.subs());
        Selection = SData.subs()->begin()->Time;

        connect(&PlayCursorUpdater, SIGNAL(timeout()), this, SLOT(updatePlayCursorPos()));
        PlayCursorUpdater.start(UpdateIntervalMs);

        // Receive move events even when no mouse button is clicked
        setMouseTracking(true);
    }

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
        return (PData.Data.size() * PData.SamplesPerPeak) / PData.SampleRate;
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

    bool MouseDown = false; // Flag indicating whether the left mouse button is currently down

    int UpdateIntervalMs = 30; // Set to 17 ms in order to try to achieve 60 fps

    QTimer PlayCursorUpdater;

private slots:
    void updatePlayCursorPos();
};

class WaveformView : public QAbstractScrollArea
{
public:
    WaveformView(AbstractRenderer *R, PeakData &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr) :
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
