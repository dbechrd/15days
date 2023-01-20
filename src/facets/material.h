#pragma once
#include "../common/basic.h"
#include "facet.h"

struct Material : public Facet {
    const char *               materialProtoKey {};
    ResourceDB::MaterialStates states           {};
};