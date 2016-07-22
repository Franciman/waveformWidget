#include "waveformview.h"

#include <QMouseEvent>

void WaveformViewport::mouseDoubleClickEvent(QMouseEvent *ev)
{
    int x = ev->pos().x();
    int y = ev->pos().y();

    // Ignore double clicks on the ruler
    if(y >= height() - RulerHeight) return;

    RangeList &SelectedRangeList = getRangeListFromPos(y);

    // If you can't edit it, you can't select it
    if(!SelectedRangeList.editable()) return;

    auto sub = SelectedRangeList.getNearestSubtitleAt(pixelToTime(x));

    if(sub == SelectedRangeList.end()) return;

    Selection.StartTime = sub->Time.StartTime;
    Selection.EndTime = sub->Time.EndTime;
    update();
}

void WaveformViewport::mousePressEvent(QMouseEvent *ev)
{
    // Ignore clicks on the ruler
    if(ev->pos().y() >= height() - RulerHeight) return;

    MouseDown = true;
    RangeList &RangeListClicked = getRangeListFromPos(ev->pos().y());
    mousePressCoolEdit(ev, RangeListClicked);
    update();
}

void WaveformViewport::mousePressCoolEdit(QMouseEvent *ev, RangeList &RangeListClicked)
{
    if(ev->button() == Qt::LeftButton)
    {
        QPoint MousePos = ev->pos();

        int NewCursorPosMs = pixelToTime(MousePos.x());

        Selection.StartTime = -1;
        CursorMs = NewCursorPosMs;
    }

}
