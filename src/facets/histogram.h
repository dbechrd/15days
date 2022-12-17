#pragma once
#include "facet.h"

struct Histogram : public Facet {
    // TODO: Is this a circular buffer? Maybe just make my own..
    std::deque<float> values{};
};