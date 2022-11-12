#pragma once
#include "facet.h"

struct Combat : public Facet {
    double attackStartedAt {};  // when current attack started
    double attackDuration  {};  // length of time current attack will last
};