#include "basic.h"
#include <cmath>

inline bool Vec3::IsZero(void)
{
    return x == 0 && y == 0 && z == 0;
}
inline bool Vec3::IsTiny(float epsilon)
{
    return fabsf(x) <= epsilon
        && fabsf(y) <= epsilon
        && fabsf(z) <= epsilon;
}