#include "waveformview.h"

#include <QWheelEvent>
#include <QMouseEvent>

#include "renderer.h"

#include <chrono>
#include <iostream>

void WaveformViewport::wheelEvent(QWheelEvent *ev)
{
    QPoint angleDelta = ev->angleDelta();

    if(angleDelta.isNull()) return;

    if(ev->modifiers() & Qt::ControlModifier)
    {
        // Horizontal zoom
        //int NewPageSizeMs;
        /*if(angleDelta.y() > 0)
        {
            NewPageSizeMs = std::round(PageSizeMs * 80.0 / 100.0);
        }
        else
        {
            NewPageSize = std::round(PageSizeMs * 125.0 / 100.0);
        }*/

    }
    else if(ev->modifiers() & Qt::ShiftModifier)
    {
        // Vertical zoom
        if(angleDelta.y() > 0)
        {
            if(VerticalScaling >= 0 && VerticalScaling <= 395) VerticalScaling += 5;
        }
        else
        {
            if(VerticalScaling >= 5 && VerticalScaling <= 400) VerticalScaling -= 5;
        }
    }
    else
    {
        // Time scrolling
        int ScrollAmount = std::round(PageSizeMs / 4.0);
        if(ScrollAmount == 0) ScrollAmount = 1;
        if(angleDelta.y() > 0)
        {
            setPosition(PositionMs + ScrollAmount);
        }
        else
        {
            setPosition(PositionMs - ScrollAmount);
        }
    }
    update();
}

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

    SData.setSelectedSubtitle(sub);

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

        // If there is a focused subtitle, adjust mouse cursor
        if(FocMode != FocusNone)
        {
            MousePos.setX(relTimeToPixel(FocusingTimeMs));
            QCursor::setPos(mapToGlobal(MousePos));
        }

        int NewCursorPosMs = pixelToTime(MousePos.x());

        if(ev->modifiers() & Qt::ShiftModifier || FocMode != FocusNone)
        {
            if(hasFocusedSubtitle() && FocusedSubtitle != SData.selectedSubtitle())
            {
                // If there is a focused subtitle, and it's different from
                // the current selected subtitle, select it
            }

            if(!hasSelection())
            {

                if(NewCursorPosMs > CursorMs)
                {
                    Selection.EndTime = NewCursorPosMs;
                    Selection.StartTime = CursorMs;
                }
                else
                {
                    Selection.EndTime = CursorMs;
                    Selection.StartTime = NewCursorPosMs;
                }
                SelectionOriginMs = Selection.StartTime;
            }
            else
            {
                if(NewCursorPosMs > Selection.StartTime + Selection.duration() / 2)
                {
                    // We are close to the end of the selection
                    Selection.EndTime = NewCursorPosMs;
                    SelectionOriginMs = Selection.StartTime;
                }
                else
                {
                    // We are close to the start of the selection
                    Selection.StartTime = NewCursorPosMs;
                    SelectionOriginMs = Selection.EndTime;
                }
            }

            if(SData.hasSelected())
            {
                // Schedule update
            }

        }
        else
        {
            // Invalidate selection
            Selection.StartTime = -1;
            SelectionOriginMs = NewCursorPosMs;
        }

        CursorMs = NewCursorPosMs;
    }

}

void WaveformViewport::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint mousePos = ev->pos();
    RangeList &RangeListFocused = getRangeListFromPos(mousePos.y());
    int Xpos = mousePos.x();
    if(MouseDown)
    {
        
    }
    else if(RangeListFocused.editable())
    {
        
    }
    else
    {
        Constrain(Xpos, 0, width() - 1);
        int CursorPosMs = pixelToTime(Xpos);

        // Subtitle focusing
        if(ev->modifiers() == 0)
        {
            // Find a subtitle under the mouse
            int RangeSelWindow = pixelToRelTime(4);
            if(RangeSelWindow < 1) RangeSelWindow = 1;


        }
    }
}

void WaveformViewport::updatePlayCursorPos()
{
    using namespace std::chrono;

    int NewPlayCursorMs = Rend->getPositionMs();
    std::cout << NewPlayCursorMs << std::endl;
    if(NewPlayCursorMs != PlayCursorMs)
    {
        PlayCursorMs = NewPlayCursorMs;
        update();
    }
    /*
    PlayCursorMs += UpdateIntervalMs;
    update();
    */
}
