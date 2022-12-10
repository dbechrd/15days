#pragma once
#include "facet.h"

struct Combat : public Facet {
    double attackStartedAt {};  // when current attack started
    double attackCooldown  {};  // length of time active attack will last
    double defendStartedAt {};  // when current block started
    double defendCooldown  {};  // length of time active block will last

    inline bool Attacking(void) {
        return attackStartedAt != 0;
    }

    inline bool Defending(void) {
        return defendStartedAt != 0;
    }

    inline bool Idle(void) {
        bool idle = !(
            Attacking() || Defending()
        );
        return idle;
    }
};