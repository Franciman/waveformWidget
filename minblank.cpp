#include "rangelist.h"

int MinBlankInfo::getStart(int minBlankTime) const
{
    if(Part == MinBlankStart)
    {
        return R->Time.StartTime - minBlankTime;
    }
    else
    {
        return R->Time.EndTime;
    }
}

int MinBlankInfo::getEnd(int minBlankTime) const
{
    if(Part == MinBlankStart)
    {
        return R->Time.StartTime;
    }
    else
    {
        return R->Time.EndTime + minBlankTime;
    }
}

int MinBlankInfo::getSnappingPoint(int minBlankTime) const
{
    if(Part == MinBlankStart)
    {
        return getStart(minBlankTime);
    }
    else
    {
        return getEnd(minBlankTime);
    }
}
