#pragma once
#include "basic.h"

struct Arena {
    size_t capacity{};
    size_t size{};
    uint8_t *data{};

    void Init(size_t bytes);
    void Destroy(void);
    void *Alloc(size_t bytes);
    void Clear(void);
};