#include "rangelist.h"

RangeLookupIterator::RangeLookupIterator(RangeList &RL, int posMs, int expandBy) :
    SearchStartAtMs(posMs),
    ExpandBy(expandBy),
    Cur(RL.subsAheadOf(SearchStartAtMs - ExpandBy)),
    Begin(RL.begin()),
    End(RL.end()),
    Result(RL.end())
{
    if(Cur == End)
    {
        --Cur;
    }
    while(Cur != Begin && Cur->Time.EndTime >= SearchStartAtMs + ExpandBy)
    {
       --Cur;
    }
    ++(*this);
}

RangeLookupIterator::RangeLookupIterator(RangeList &RL) :
    SearchStartAtMs(0),
    ExpandBy(0),
    Cur(RL.end()),
    Begin(RL.begin()),
    End(RL.end()),
    Result(RL.end())
{ }

RangeLookupIterator &RangeLookupIterator::operator ++()
{
    Result = End;
    while(Cur >= Begin && Cur != End)
    {
        std::vector<SrtSubtitle>::iterator Tmp = Cur;
        ++Cur;
        if(Tmp->Time.StartTime > SearchStartAtMs + ExpandBy)
        {
            break;
        }
        else if(Tmp->Time.StartTime <= SearchStartAtMs + ExpandBy &&
                Tmp->Time.EndTime >= SearchStartAtMs - ExpandBy)
        {
            Result = Tmp;
            break;
        }
    }
    return *this;
}
