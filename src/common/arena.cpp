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
    if (size + bytes > capacity) {
        size_t newCap = capacity * 2;
        uint8_t *newData = (uint8_t *)realloc(data, newCap);
        if (!newData) {
            printf("ERROR: Failed to resize depot frame arena\n");
            return 0;
        }
        capacity = newCap;
        data = newData;
    }
    void *ptr = data + size;
    size += bytes;
    return ptr;
}

void Arena::Clear(void)
{
    memset(data, 0, capacity);
    size = 0;
}
