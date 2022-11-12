#pragma once
#include <cstdint>

typedef uint32_t UID;

struct Color {
    uint8_t r{};
    uint8_t g{};
    uint8_t b{};
    uint8_t a{};
};

struct Rect {
    float x{};
    float y{};
    float w{};
    float h{};
};

// TODO: Move these to maths.h
struct Vec2 {
    float x{};
    float y{};
};

struct Vec3 {
    float x{};
    float y{};
    float z{};

    bool IsZero(void);
    bool IsTiny(float epsilon = 0.0001f);
};