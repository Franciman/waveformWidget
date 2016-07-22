#ifndef SRTPARSER_H
#define SRTPARSER_H

#include <QTextStream>
#include <QString>
#include <QFile>
#include <QList>

#include <vector>

#include "srtsubtitle.h"


class SrtParseError
{
    const char *What;
public:
    SrtParseError(const char *what) :
        What(what)
    { }

    const char *what() const
    {
        return What;
    }
};

class SrtParser
{
    QTextStream InputStream;
    QString CurrLine;
    QString::iterator CurrLineChar;
    QString::iterator CurrLineEnd;

    void nextLine()
    {
        CurrLine = InputStream.readLine();
        CurrLineChar = CurrLine.begin();
        CurrLineEnd = CurrLine.end();
    }

    void advanceOnLine()
    {
        ++CurrLineChar;
    }

    bool moreCharsOnLine() const
    {
        return CurrLineChar != CurrLineEnd;
    }

public:
    SrtParser(QIODevice *file) :
        InputStream(file)
    { }

    std::vector<SrtSubtitle> parseSubs()
    {
        std::vector<SrtSubtitle> Result;
        while(!InputStream.atEnd())
        {
            Result.push_back(parseSub());
        }
        return Result;
    }

    SrtSubtitle parseSub()
    {
        SrtSubtitle Result;
        nextLine();
        Result.Number = parseNumber();
        nextLine();
        parseTimeSignature(Result.Time.StartTime, Result.Time.EndTime);
        Result.Text = parseContent();
        return Result;
    }

private:
    unsigned int parseNumber()
    {
        unsigned int Result = 0;
        if(CurrLine.isNull())
        {
            throw SrtParseError("Expected subtitle number, got nothing");
        }
        for(; CurrLineChar != CurrLineEnd; ++CurrLineChar)
        {
            if(!CurrLineChar->isDigit()) throw SrtParseError("Expected number");
            else
            {
                Result *= 10;
                Result += CurrLineChar->digitValue();
            }
        }
        return Result;
    }

    void parseTimeSignature(int &start, int &end)
    {
        if(CurrLine.isNull())
        {
            throw SrtParseError("Expected timing signature");
        }
        start = parseTime();
        expect(" --> ");
        end = parseTime();
        if(moreCharsOnLine()) throw SrtParseError("Unexpected chars after time signature");
    }

    int parseTime()
    {
        SrtTime Result;
        Result.Hour = parseDigits(2);
        expect(':');
        Result.Minute = parseDigits(2);
        expect(':');
        Result.Seconds = parseDigits(2);
        expect(',');
        Result.Milliseconds = parseDigits(3);
        return Result.toMs();
    }

    unsigned int parseDigits(int numDigits)
    {
        unsigned int result = 0;
        for(int i = 0; i < numDigits; i++)
        {
           if(!moreCharsOnLine() || !CurrLineChar->isDigit()) throw SrtParseError("Expected digit but could not get it");
           result *= 10;
           result += CurrLineChar->digitValue();
           advanceOnLine();
        }
        return result;
    }

    void expect(const char *str)
    {
        for(const char *str_ptr = str; *str_ptr != '\0'; ++str_ptr)
        {
            expect(*str_ptr);
        }
    }

    void expect(char ch)
    {
        if(!moreCharsOnLine() || *CurrLineChar != ch) throw SrtParseError("Expected char but could not get it");
        else advanceOnLine();
    }

    QString parseContent()
    {
        QString result;
        nextLine();

        // TODO: make sure the format actually says so
        // The first line can be empty
        if(CurrLine.isEmpty()) nextLine();

        for(; !CurrLine.isEmpty(); nextLine())
        {
            result.append(CurrLine);
            result.append('\n');
        }
        return result.trimmed();
    }
};

#endif // SRTPARSER_H
