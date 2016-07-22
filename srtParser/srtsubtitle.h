#ifndef SRTSUBTITLE_H
#define SRTSUBTITLE_H

#include <QObject>
#include <QAbstractItemModel>
#include <QHash>
#include <QVector>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <stdio.h>

// #include "waveformView/timemstoshortstring.h"


struct SrtTime
{
    int Hour;
    int Minute;
    int Seconds;
    int Milliseconds;

    double toDouble() const
    {
       double Result = 0.0;
       Result += static_cast<double>(Milliseconds) / 1000.0;
       Result += static_cast<double>(Seconds);
       Result += static_cast<double>(Minute) * 60.0;
       Result += static_cast<double>(Hour) * 3600.0;
       return Result;
    }

    int toMs() const
    {
        int Result = 0;
        Result += Hour * 60;
        Result += Minute;
        Result *= 60;
        Result += Seconds;
        Result *= 1000;
        Result += Milliseconds;
        return Result;
    }

    SrtTime &operator=(int timeMs)
    {
        Milliseconds = timeMs / 1000;
        Hour = Milliseconds / 3600;
        Minute = (Milliseconds / 60) - (Hour * 60);
        Seconds = Milliseconds / 60;
        Milliseconds = (timeMs - (Hour * 3600 * 1000) - (Minute * 60 * 1000) - (Seconds * 1000));

        return *this;
    }

    /*QString toString() const
    {
        char buff[16];
        sprintf(buff, "%02d:%02d:%02d,%03d", Hour, Minute, Seconds, Milliseconds);
        return QString(buff);
    }*/

    bool operator==(const SrtTime &other) const
    {
        return Hour == other.Hour && Minute == other.Minute && Seconds == other.Seconds && Milliseconds == other.Milliseconds;
    }
};

struct Range
{
    int StartTime;
    int EndTime;

    int duration() const
    {
        return EndTime - StartTime;
    }


    bool operator==(const Range &other) const
    {
        return StartTime == other.StartTime && EndTime == other.EndTime;
    }

    bool operator!=(const Range &other) const
    {
        return !(*this == other);
    }
};



struct SrtSubtitle
{
    unsigned int Number;
    Range Time;
    QString Text;

    bool equalTimings(const SrtSubtitle &other) const
    {
        return other.Time.StartTime == Time.StartTime && other.Time.EndTime == Time.EndTime;
    }

    void writeToFile(QIODevice *device)
    {
        QTextStream stream(device);
        stream << Number << "\n";
        stream << msToString(Time.StartTime) << " --> " << msToString(Time.EndTime) << "\n";
        stream << Text << "\n\n";
    }
private:
    static QString msToString(int ms)
    {
        int millisec = ms % 1000;
        ms = (ms - millisec) / 1000;
        int sec = ms % 60;
        ms = (ms - sec) / 60;
        int minute = ms % 60;
        int hour = (ms - minute) / 60;

        return QString::asprintf("%02d:%02d:%02d,%03d", hour, minute, sec, millisec);
    }
};

#endif // SRTSUBTITLE_H
