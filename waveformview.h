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

struct Peak
{
    int Min;
    int Max;
};

struct PeakData
{
    std::vector<Peak> Peaks;
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
    // This list is always sorted based on time
    // So that it can be searched in log(n) time
    RangeList Subs;
    RangeList *VO;
public:
    typedef RangeList::iterator iterator;

    SubtitleData(RangeList &&subs, RangeList *vo = nullptr) :
        Subs(std::move(subs)),
        VO(vo)
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
};

class WaveformViewport : public QOpenGLWidget
{
    Q_OBJECT

    // Model data ----
    PeakData PData;
    SubtitleData SData;
    // ---------------

    std::vector<RangeList *> DisplayRangeLists;
public:
    WaveformViewport(PeakData &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr) :
        QOpenGLWidget(parent),
        PData(std::move(pdata)),
        SData(std::move(sdata))
    {
        if(SData.hasVO())
        {
            DisplayRangeLists.push_back(SData.vo());
        }
        DisplayRangeLists.push_back(SData.subs());
        Selection = SData.subs()->begin()->Time;

        connect(&PlayCursorUpdater, SIGNAL(timeout()), this, SLOT(updatePlayCursorPos()));
        PlayCursorUpdater.start(UpdateIntervalMs);
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
        return (PData.Peaks.size() * PData.SamplesPerPeak) / PData.SampleRate;
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
        QPainter painter(this);
        paintWav(painter);
        paintRuler(painter);
        paintRangeLists(painter);
        paintSelection(painter);
        paintCursor(painter);
        paintPlayCursor(painter);
    }

    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;

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

    inline unsigned int pixelToTime(int Pixel) const
    {
        double MsPerPixel = double(PageSizeMs) / width();
        return std::round(MsPerPixel * Pixel) + PositionMs;
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

    bool MouseDown = false; // Flag indicating whether the left mouse button is currently down

    int UpdateIntervalMs = 17;

    QTimer PlayCursorUpdater;

private slots:
    void updatePlayCursorPos()
    {
        PlayCursorMs += UpdateIntervalMs;
        update();
    }

};

class WaveformView : public QAbstractScrollArea
{
public:
    WaveformView(PeakData &&pdata, SubtitleData &&sdata, QWidget *parent = nullptr) :
        QAbstractScrollArea(parent),
        Viewport(new WaveformViewport(std::move(pdata), std::move(sdata), this))
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
