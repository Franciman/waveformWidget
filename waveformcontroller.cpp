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

    auto sub = SelectedRangeList.getSubtitleAt(pixelToTime(x));

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

        // If there is a focused time, adjust mouse cursor to it
        if(FocusMode != FocusNone)
        {
            MousePos.setX(relTimeToPixel(FocusedTimeMs));
            QCursor::setPos(mapToGlobal(MousePos));
        }

        int NewCursorPosMs = pixelToTime(MousePos.x());

        // Adjust cursor to Snapping point
        // Unless Ctrl is pressed
        if(!(ev->modifiers() & Qt::ControlModifier) && EnableSnapping)
        {
            int SnappingPos = findCorrectedSnappingPoint(NewCursorPosMs, RangeListClicked);
            if(SnappingPos != -1)
            {
                NewCursorPosMs = SnappingPos;
            }
        }

        // If Shift is pressed, modify current selection
        // Or if no selection is present, create a new one
        // that goes from min(NewCursrPosMs, CursorMs) to max(NewCursorPosMs, CursorMs)
        // Also Set SelectionOriginMs properly
        if(ev->modifiers() & Qt::ShiftModifier || FocusMode != FocusNone)
        {
            // If click happens near start or end of subtitle,
            // select it
            if(hasFocusedSubtitle())
            {
                SData.setSelectedSubtitle(FocusedSubtitle);
                Selection = FocusedSubtitle->Time;
            }
            if(!hasSelection())
            {
                // Create new selection
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
                    // so its EndTime is going to be changed
                    Selection.EndTime = NewCursorPosMs;
                    SelectionOriginMs = Selection.StartTime;
                }
                else
                {
                    // We are close to the start of the selection
                    // so its StartTime is going to be changed
                    Selection.StartTime = NewCursorPosMs;
                    SelectionOriginMs = Selection.EndTime;
                }

                // If the selection was a Subtitle,
                // Update it and schedule sorting
                if(SData.hasSelected())
                {
                    // Schedule update
                    SubChanged = true;
                    SData.selectedSubtitle()->Time.StartTime = Selection.StartTime;
                    SData.selectedSubtitle()->Time.EndTime = Selection.EndTime;
                }
            }
        }
        else
        {
            // Invalidate selection
            Selection.StartTime = -1;
            SelectionOriginMs = NewCursorPosMs;
            SData.setSelectedSubtitle(SData.end());
        }

        if(EnableMouseAntiOverlapping)
        {
            // Avoid overlapping
            MinSelTime = -1;
            MaxSelTime = -1;

            auto insertPos = RangeListClicked.subsAheadOf(NewCursorPosMs);
            if(insertPos >= RangeListClicked.begin())
            {
                if(SData.hasSelected())
                {
                    if(NewCursorPosMs == SData.selectedSubtitle()->Time.StartTime)
                    {
                        if(insertPos != RangeListClicked.begin())
                        {
                            MinSelTime = (insertPos - 1)->Time.EndTime + 1;
                        }
                        MaxSelTime = SData.selectedSubtitle()->Time.EndTime - 1;
                    }
                    else
                    {
                        MinSelTime = SData.selectedSubtitle()->Time.StartTime + 1;
                        if(insertPos != SData.end())
                        {
                            MaxSelTime = insertPos->Time.StartTime - 1;
                        }
                    }
                }
                else
                {
                    if(insertPos != RangeListClicked.begin() &&
                            NewCursorPosMs >= (insertPos - 1)->Time.StartTime &&
                            NewCursorPosMs <= (insertPos -1)->Time.EndTime)
                    {
                        // Selection only inside subtitle range
                    }
                    else
                    {
                        if(insertPos != RangeListClicked.begin())
                        {
                            MinSelTime = (insertPos - 1)->Time.EndTime + 1;
                        }
                        if(insertPos != RangeListClicked.end())
                        {
                            MaxSelTime = insertPos->Time.StartTime - 1;
                        }
                    }
                }
            }
            // We don't clip mouse cursor because for now
            // Qt hasn't got an easy way to do that, and it seems
            // it's not really useful
        }

        // If there is no focused time, update cursor position
        if(FocusMode == FocusNone)
        {
            CursorMs = NewCursorPosMs;
        }
        update();
    }

}

void WaveformViewport::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint mousePos = ev->pos();
    RangeList &RangeListFocused = getRangeListFromPos(mousePos.y());
    int Xpos = mousePos.x();
    if(MouseDown)
    {
        if(ev->buttons() & Qt::LeftButton)
        {
            Constrain(Xpos, 0, width() - 1);
            int NewCursorPosMs = pixelToTime(Xpos);

            // Avoid overlapping
            if(MinSelTime != -1)
            {
                if(NewCursorPosMs < MinSelTime) NewCursorPosMs = MinSelTime;
            }
            if(MaxSelTime != -1)
            {
                if(NewCursorPosMs > MaxSelTime) NewCursorPosMs = MaxSelTime;
            }

            // Snapping
            if(!(ev->modifiers() & Qt::ControlModifier) && EnableSnapping)
            {
                int SnappingPos = findCorrectedSnappingPoint(NewCursorPosMs, RangeListFocused);
                if(SnappingPos != -1)
                {
                    NewCursorPosMs = SnappingPos;
                }
            }

            if(SelectionOriginMs != -1 && SelectionOriginMs != NewCursorPosMs)
            {
                // Update selection
                if(NewCursorPosMs > SelectionOriginMs)
                {
                    Selection.StartTime = SelectionOriginMs;
                    Selection.EndTime = NewCursorPosMs;
                }
                else
                {
                    Selection.StartTime = NewCursorPosMs;
                    Selection.EndTime = SelectionOriginMs;
                }
                if(SData.hasSelected())
                {
                    // We have changed a subtitle
                    SubChanged = true;
                    SData.selectedSubtitle()->Time = Selection;
                }
            }

            if(CursorMs != NewCursorPosMs) //&& FocusingMode == FocusNone)
            {
                CursorMs = NewCursorPosMs;
            }

            update();
        }
    }
    else if(RangeListFocused.editable())
    {
        // Dynamic selection
        Constrain(Xpos, 0, width() - 1);
        int CursorPosMs = pixelToTime(Xpos);

        // Ignore movements outside the waveform
        if(mousePos.y() <= 0 || mousePos.y() > height() - RulerHeight - 1)
            return;

        // Have a tolerance of 4ms
        int ToleranceFromBorder = pixelToRelTime(4);

        // First search for subtitle under the mouse
        if(!findFocusingSubtitle(CursorPosMs, RangeListFocused, ToleranceFromBorder, 0))
        {
            // If no subtitle is under the mouse

            // Have a tolerance of 2ms
            ToleranceFromBorder = pixelToRelTime(2);

            // Find subtitles that are at most 2ms away from the mouse cursor
            int ToleranceFromPosition = pixelToRelTime(2);
            if(ToleranceFromPosition < 1) ToleranceFromPosition = 1;

            if(!findFocusingSubtitle(CursorPosMs, RangeListFocused, ToleranceFromBorder, ToleranceFromPosition))
            {
                // If no subtitle is near the mouse cursor
                // Check if there is selection
                if(hasSelection())
                {
                    // Check if we're near the selection
                    ToleranceFromBorder = pixelToRelTime(4);

                    FocusingMode NewMode;
                    int NewTime;
                    if(checkRangeForFocusing(Selection, CursorPosMs, ToleranceFromBorder, NewMode, NewTime))
                    {
                        setCursor(Qt::SizeHorCursor);
                        FocusMode = NewMode;
                        FocusedTimeMs = NewTime;
                        // Invalidate focused subtitle,
                        // there is no subtitle focused
                        OldFocusedSubtitle = FocusedSubtitle;
                        FocusedSubtitle = SData.end();
                        if(OldFocusedSubtitle != FocusedSubtitle)
                        {
                            update();
                        }
                        return;
                    }
                }
            }
            else return;
        }
        else
        {
            return;
        }

        // Set MinBlank

        // If no focused subtitle o selection has been found,
        // reset focusing to none
        setCursor(Qt::ArrowCursor);
        FocusMode = FocusNone;
        OldFocusedSubtitle = FocusedSubtitle;
        FocusedSubtitle = SData.end();
        if(OldFocusedSubtitle != FocusedSubtitle)
        {
            update();
        }

    }
    else
    {
        // reset focusing to none
        setCursor(Qt::ArrowCursor);
        FocusMode = FocusNone;
        OldFocusedSubtitle = FocusedSubtitle;
        FocusedSubtitle = SData.end();
        if(OldFocusedSubtitle != FocusedSubtitle)
        {
            update();
        }

    }
}

void WaveformViewport::mouseReleaseEvent(QMouseEvent *ev)
{
    RangeList &RL = getRangeListFromPos(ev->pos().y());

    if(Selection.duration() < 40 && !SData.hasSelected() && !hasFocusedSubtitle())
    {
        // Clear selection
        Selection.StartTime = -1;
        update();
    }

    if(SubChanged)
    {
        RL.sortSubs();
        SubChanged = false;
    }

    // Invalidate everything
    SelectionOriginMs = -1;
    setCursor(Qt::ArrowCursor);
    MouseDown = false;
    FocusMode = FocusNone;
    FocusedSubtitle = SData.end();
    MinSelTime = -1;
    MaxSelTime = -1;
}

void WaveformViewport::updatePlayCursorPos()
{
    using namespace std::chrono;

    updatePlayCursorPos(Rend->getPositionMs());
}

void WaveformViewport::updatePlayCursorPos(int PosMs)
{
    // We check if the new Pos is at least
    // 10ms away from previous value in order to avoid too rapid updates
    if(std::abs(PosMs - PlayCursorMs) >= 10)
    {
        PlayCursorMs = PosMs;
        update();
    }
}
