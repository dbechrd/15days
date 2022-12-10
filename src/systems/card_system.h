#pragma once
#include "../common/basic.h"

struct CardSystem {
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);

private:
    UID FindCardAtScreenPos(Depot &depot, int x, int y, vec2 *offset);
};