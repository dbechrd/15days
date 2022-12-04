#include "text.h"
#include <cstring>

bool Text::isDirty(void)
{
    return
        font != cache.font ||
        !v4_equals(&color, &cache.color) ||
        strcmp(str, cache.str);
}