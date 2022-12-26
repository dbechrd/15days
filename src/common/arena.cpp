#include "arena.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void Arena::Init(size_t bytes)
{
    data = (uint8_t *)calloc(bytes, sizeof(*data));
    if (data) {
        capacity = bytes;
    } else {
        printf("ERROR: Failed to allocate depot frame arena\n");
    }
}

void Arena::Destroy(void)
{
    free(data);
    *this = {};
}

void *Arena::Alloc(size_t bytes)
{
    DLB_ASSERT(capacity > 0);
    if (used + bytes > capacity) {
#if 0
        if (canResize) {
            size_t newCap = capacity * 2;
            uint8_t *newData = (uint8_t *)realloc(data, newCap);
            if (!newData) {
                printf("ERROR: Failed to resize depot frame arena\n");
                return 0;
            }
            capacity = newCap;
            data = newData;
        } else {
            DLB_ASSERT(!"You dun ran outta space, bruv");
            return 0;
        }
#else
        DLB_ASSERT(!"You dun ran outta space, bruv");
        return 0;
#endif
    }
    void *ptr = data + used;
    used += bytes;
    return ptr;
}

void Arena::Reset(void)
{
    memset(data, 0, capacity);
    used = 0;
}
