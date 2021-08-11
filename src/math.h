#ifndef MATH_H

#include "platform.h"

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




#define MATH_H
#endif
