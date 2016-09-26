#ifndef MODEL_H
#define MODEL_H

#include "rangelist.h"

class SubtitleData
{
public:
    typedef RangeList::iterator iterator;

private:
    // This list is always sorted based on time
    // So that it can be searched in log(n) time
    RangeList Subs;
    RangeList *VO;
    iterator Selected;
public:

    SubtitleData(RangeList &&subs, RangeList *vo = nullptr) :
        Subs(std::move(subs)),
        VO(vo),
        Selected(Subs.end())
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

    void setSelectedSubtitle(iterator sub)
    {
        Selected = sub;
    }

    iterator selectedSubtitle()
    {
        return Selected;
    }

    bool hasSelected()
    {
        return Selected != Subs.end();
    }
};


#endif // MODEL_H
