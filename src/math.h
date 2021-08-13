#ifndef MATH_H

#include "platform.h"
#include <math.h>

struct v3
{
    union
    {
        r32 V[3];
        struct
        {
            r32 x,y,z;
        };
    };
};

#define V30 V3(0.0f)
inline v3
V3(r32 r)
{
    return { r, r, r};
}

inline v3
V3(r32 x,r32 y, r32 z)
{
    return { x, y, z};
}

inline v3
operator /(r32 d, v3 v)
{
    v3 Result = {d / v.x, d / v.y, d / v.z };

    return Result;
}

inline v3
operator /(v3 V, r32 R)
{
    r32 OneOverR = 1.0f / R;
    v3 Result = { V.x * R, V.y * R, V.z * R };

    return Result;
}

inline v3
operator -(v3 a, v3 b)
{
    v3 Result = {a.x - b.x, a.y - b.y, a.z - b.z };

    return Result;
}

inline v3
operator +(v3 a, v3 b)
{
    v3 Result = {a.x + b.x, a.y + b.y, a.z + b.z };

    return Result;
}

inline v3
operator +=(v3 &a, v3 b)
{
    a = a + b;

    return a;
}
inline v3
operator -=(v3 &a, v3 b)
{
    a = a - b;

    return a;
}

// dot product
inline r32
Inner(v3 A, v3 B)
{
    r32 Result =  A.x * B.x + A.y * B.y + A.z * B.z;

    return Result;
}

inline r32
LengthSqr(v3 V)
{
    r32 Result = Inner(V, V);

    return Result;
}

inline r32
Length(v3 V)
{
    r32 Result = sqrtf(LengthSqr(V));
    return Result;
}

inline v3
UnitLength(v3 V)
{
    v3 Result = V / LengthSqr(V);
    return Result;
}




#define MATH_H
#endif
