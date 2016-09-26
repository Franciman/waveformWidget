#ifndef PEAKS_H
#define PEAKS_H

#include <cstdint>
#include <vector>

using std::int32_t;

class Peak
{
    int32_t Min;
    int32_t Max;
public:
    Peak() { }
    Peak(int32_t min, int32_t max) :
        Min(min),
        Max(max)
    { }

    int32_t min() const
    {
        return Min;
    }

    int32_t max() const
    {
        return Max;
    }

    void min(int32_t value)
    {
        Min = value;
    }

    void max(int32_t value)
    {
        Max = value;
    }
};

class Peaks
{
    std::vector<Peak> PeakList;
    int32_t MinPeak;
    int32_t MaxPeak;
    int SamplesPerPeak;
    int SampleRate;
public:
    Peaks() { }
    Peaks(int samplesPerPeak, int sampleRate)
    {
        SamplesPerPeak = samplesPerPeak;
        SampleRate = sampleRate;
    }

    Peaks(Peaks &&other) :
        PeakList(std::move(other.PeakList)),
        MinPeak(other.MinPeak),
        MaxPeak(other.MaxPeak),
        SamplesPerPeak(other.SamplesPerPeak),
        SampleRate(other.SampleRate)
    { }
    Peaks(std::vector<Peak> &&peakList, int32_t minPeak, int32_t maxPeak, int samplesPerPeak, int sampleRate) :
        PeakList(std::move(peakList)),
        MinPeak(minPeak),
        MaxPeak(maxPeak),
        SamplesPerPeak(samplesPerPeak),
        SampleRate(sampleRate)
    { }

    Peaks &operator=(Peaks &&other)
    {
        PeakList = std::move(other.PeakList);
        MinPeak = other.MinPeak;
        MaxPeak = other.MaxPeak;
        SamplesPerPeak = other.SamplesPerPeak;
        SampleRate = other.SampleRate;
        return *this;
    }

    void samplesPerPeak(int value)
    {
        SamplesPerPeak = value;
    }

    int samplesPerPeak() const
    {
        return SamplesPerPeak;
    }

    void sampleRate(int value)
    {
        SampleRate = value;
    }

    int sampleRate() const
    {
        return SampleRate;
    }

    int32_t minPeak() const
    {
        return MinPeak;
    }

    int32_t maxPeak() const
    {
        return MaxPeak;
    }

    std::vector<Peak>::iterator peaks_begin()
    {
        return PeakList.begin();
    }

    std::vector<Peak>::iterator peaks_end()
    {
        return PeakList.end();
    }

    bool empty() const
    {
        return PeakList.empty();
    }

    void addPeak(Peak &P)
    {
        PeakList.emplace_back(P.min(), P.max());
    }

    void updateMaxPeak(int32_t newMax)
    {
        MaxPeak = newMax;
    }

    void updateMinPeak(int32_t newMin)
    {
        MinPeak = newMin;
    }

    std::vector<Peak>::size_type peaksNumber() const
    {
        return PeakList.size();
    }

    Peak &operator[](std::size_t num)
    {
        return PeakList[num];
    }

    const Peak &operator[](std::size_t num) const
    {
        return PeakList[num];
    }

};


#endif // PEAKS_H
