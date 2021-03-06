/*
Copyright (c) 2008 Joshua Tippetts

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef UTILITY_H
#define UTILITY_H
#include <cmath>
#include "../types.h"

template<typename TYPE> TYPE clamp(TYPE v, TYPE l, TYPE h)
{
    if(v<l) v=l;
    if(v>h) v=h;

    return v;
};

template<typename TYPE> TYPE lerp(ANLFloatType t, TYPE a, TYPE b)
{
    return a+t*(b-a);
}

template<typename TYPE> bool isPowerOf2(TYPE n)
{
    return !((n-1) & n);
}

inline ANLFloatType hermite_blend(ANLFloatType t)
{
    return (t*t*(3-2*t));
}

inline ANLFloatType quintic_blend(ANLFloatType t)
{
    return t*t*t*(t*(t*6-15)+10);
}

inline int fast_floor(ANLFloatType t)
{
    return (t>0 ? (int)t : (int)t - 1);
}

inline ANLFloatType array_dot(ANLFloatType *arr, ANLFloatType a, ANLFloatType b)
{
    return a*arr[0] + b*arr[1];
}

inline ANLFloatType array_dot3(ANLFloatType *arr, ANLFloatType a, ANLFloatType b, ANLFloatType c)
{
    return a*arr[0] + b*arr[1] + c*arr[2];
}

inline ANLFloatType bias(ANLFloatType b, ANLFloatType t)
{
    return pow(t, log(b)/log(0.5));
}

inline ANLFloatType gain(ANLFloatType g, ANLFloatType t)
{
    if(t<0.5)
    {
        return bias(1.0-g, 2.0*t)/2.0;
    }
    else
    {
        return 1.0 - bias(1.0-g, 2.0 - 2.0*t)/2.0;
    }
}

#endif

