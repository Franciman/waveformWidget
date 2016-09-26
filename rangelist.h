#ifndef RANGELIST_H
#define RANGELIST_H

#include "srtParser/srtsubtitle.h"

class RangeList;

class RangeLookupIterator
{
    int SearchStartAtMs;
    int ExpandBy;
    std::vector<SrtSubtitle>::iterator Cur;
    std::vector<SrtSubtitle>::iterator Begin;
    std::vector<SrtSubtitle>::iterator End;
    std::vector<SrtSubtitle>::iterator Result;

public:
    typedef std::vector<SrtSubtitle>::iterator iterator;

private:
    // Create begin iterator
    RangeLookupIterator(RangeList &RL, int PosMs, int expandBy = 0);

    // Create end iterator
    RangeLookupIterator(RangeList &RL);

public:
    RangeLookupIterator &operator++();

    std::vector<SrtSubtitle>::iterator::reference operator*()
    {
        return *Result;
    }

    std::vector<SrtSubtitle>::iterator::pointer operator->()
    {
        return Result.operator ->();
    }

    bool operator==(const RangeLookupIterator &other) const
    {
        return Result == other.Result;
    }

    bool operator!=(const RangeLookupIterator &other) const
    {
        return !(*this == other);
    }

    operator iterator()
    {
        return Result;
    }

    friend class RangeList;
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

    RangeLookupIterator first_at(int PosMs, int expandBy = 0)
    {
        return RangeLookupIterator(*this, PosMs, expandBy);
    }

    RangeLookupIterator end_search()
    {
        return RangeLookupIterator(*this);
    }

    const SrtSubtitle &operator[](size_type index) const
    {
        return Subs[index];
    }

    void addSubtitle(SrtSubtitle &&sub);

    void addSubtitleAtEnd(SrtSubtitle &&sub);


    iterator getInsertPos(const Range &R)
    {
        return std::lower_bound(Subs.begin(), Subs.end(), R, [](const SrtSubtitle &Sub, const Range &r)
        {
            return Sub.Time.StartTime < r.StartTime ||
                    (Sub.Time.StartTime == r.StartTime && Sub.Time.EndTime < r.EndTime);
        });
    }

    iterator subsAheadOf(int StartPosMs)
    {
        /*return std::lower_bound(Subs.begin(), Subs.end(), StartPosMs, [](const SrtSubtitle &Sub, int PosMs) -> bool
        {
            return Sub.Time.EndTime < PosMs;
        });*/
        return getInsertPos(Range {StartPosMs, -1});
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

    iterator getSubtitleAt(int PosMs)
    {
        auto it = subsAheadOf(PosMs);
        auto result = Subs.end();
        while(it >= Subs.begin())
        {
            if(it->Time.StartTime <= PosMs && PosMs <= it->Time.EndTime)
            {
                result = it;
            }
            --it;
        }
        return result;
    }

};

enum MinBlankInfoPart
{
    MinBlankStart,
    MinBlankEnd,
    MinBlankInvalid
};

class MinBlankInfo
{
    bool Exists;
    RangeList::iterator R;
    MinBlankInfoPart Part;

public:

    MinBlankInfo() :
        Exists(false)
    { }

    bool exists() const
    {
        return Exists;
    }

    bool setInfo(RangeList::iterator NewRange, MinBlankInfoPart NewPart)
    {
        bool Res = false;
        if(R != NewRange)
        {
            R = NewRange;
            Res = true;
        }
        if(Part != NewPart)
        {
            Part = NewPart;
            Res = true;
        }
        return Res;
    }

    int getStart(int minBlankTime) const;
    int getEnd(int minBlankTime) const;
    int getSnappingPoint(int MinBlankTime) const;
};


#endif // RANGELIST_H
