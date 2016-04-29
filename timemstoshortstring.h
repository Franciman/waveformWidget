#ifndef TIMEMSTOSHORTSTRING_H
#define TIMEMSTOSHORTSTRING_H

#include <QString>

QString timeMsToShortString(int timeMs, int Precision)
{
    int hour, min, sec, ms;
    ms = timeMs / 1000;
    hour = ms / 3600;
    min = (ms / 60) - (hour * 60);
    sec = ms / 60;
    ms = (timeMs - (hour * 3600 * 1000) - (min * 60 * 1000) - (sec * 1000)) / Precision;

    QString Result;
    if(hour > 0)
    {
        if(ms > 0)
        {
           Result = QString::asprintf("%d:%.2d:%.2d.%d", hour, min, sec, ms);
        }
        else
        {
            Result = QString::asprintf("%d:%.2d:%.2d", hour, min, sec);
        }
    }
    else if(min > 0)
    {
        if(ms > 0)
        {
            Result = QString::asprintf("%d:%.2d.%d", min, sec, ms);
        }
        else
        {
            Result = QString::asprintf("%d:%.2d", min, sec);
        }
    }
    else
    {
        Result = QString::asprintf("%d.%d", sec, ms);
    }
    return Result;
}

#endif // TIMEMSTOSHORTSTRING_H
