#include "waveformview.h"

bool WaveformViewport::findFocusingSubtitle(int CursorPosMs, RangeList &RL, int ToleranceFromBorder, int PositionTolerance)
{
    FocusingMode NewFocusMode = FocusNone;
    int NewFocusedTime = -1;
    bool Result = false;

    if(ToleranceFromBorder < 1) ToleranceFromBorder = 1;

    auto it = RL.first_at(CursorPosMs, PositionTolerance);
    while(it != RL.end_search())
    {
        if(checkRangeForFocusing(it->Time, CursorPosMs, ToleranceFromBorder, NewFocusMode, NewFocusedTime))
        {
            Result = true;
            break;
        }
        ++it;
    }

    if(NewFocusMode != FocusNone)
    {
        setCursor(Qt::SizeHorCursor);
        FocusMode = NewFocusMode;
        OldFocusedSubtitle = FocusedSubtitle;
        FocusedTimeMs = NewFocusedTime;
        if(it->Time != Selection)
        {
            FocusedSubtitle = it;
        }
        else if(SData.hasSelected())
        {
            FocusedSubtitle = SData.selectedSubtitle();
        }
        else
        {
            // This should be unreachable, I guess
            FocusedSubtitle = SData.end();
        }
    }

    if(FocusedSubtitle != OldFocusedSubtitle)
    {
        if(hasFocusedSubtitle() && FocusMode != FocusNone)
        {
            // TODO
        }
        update();
    }

    return Result;
}

int WaveformViewport::findSnappingPoint(int PosMs, RangeList &RL)
{
    constexpr int SNAPPING_DISTANCE_PIXEL = 8;

    int Candidate = -1;

    int SnappingDistanceTime = pixelToRelTime(SNAPPING_DISTANCE_PIXEL);

    if(MinimumBlankMs > 0)
    {
        if(Info1.exists())
        {
            Candidate = Info1.getSnappingPoint(MinimumBlankMs);
            if(std::abs(Candidate - PosMs) <= SnappingDistanceTime)
            {
                return Candidate;
            }
        }

        if(Info2.exists())
        {
            Candidate = Info2.getSnappingPoint(MinimumBlankMs);
            if(std::abs(Candidate - PosMs) <= SnappingDistanceTime)
            {
                return Candidate;
            }
        }
    }

    // TODO: Add SceneChanges

    return -1;
}

int WaveformViewport::findCorrectedSnappingPoint(int PosMs, RangeList &RL)
{
    int Result = findSnappingPoint(PosMs, RL);
    if(Result == -1)
    {
        return -1;
    }

    // Do a correction, if there are antioverlapping infos
    if(MinSelTime != -1 && Result < MinSelTime)
        Result = MinSelTime;

    if(MaxSelTime != -1 && Result > MaxSelTime)
        Result = MaxSelTime;

    return Result;
}


bool WaveformViewport::checkRangeForFocusing(Range &R, int CursorPosMs, int ToleranceFromBorder, FocusingMode &NewMode, int &NewFocusTime)
{
    if(R.duration() / ToleranceFromBorder > 2)
    {
        if(std::abs(CursorPosMs - R.StartTime) < ToleranceFromBorder)
        {
            NewMode = FocusBegin;
            NewFocusTime = R.StartTime;
            return true;
        }
        else if(std::abs(CursorPosMs - R.EndTime) < ToleranceFromBorder)
        {
            NewMode = FocusEnd;
            NewFocusTime = R.EndTime;
            return true;
        }
    }
    return false;
}
