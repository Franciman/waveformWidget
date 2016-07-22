#ifndef CONSTRAIN_H
#define CONSTRAIN_H

template<class T>
void Constrain(T &value, const T &minVal, const T &maxVal)
{
    if(value < minVal) value = minVal;
    else if(value > maxVal) value = maxVal;
}

#endif // CONSTRAIN_H
