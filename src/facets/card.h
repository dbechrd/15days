#pragma once
#include "../common/basic.h"
#include "facet.h"
#include "material.h"

enum CardType {
    CardType_Card,
    CardType_Deck,
};

struct Card : public Facet {
    CardType     cardType     {};
    UID          stackParent  {};
    UID          stackChild   {};
    bool         wantsToStack {};
    const char * cardProto    {};
    double       noClickUntil {};  // timestamp of when next click is allowed on card

    union {
        struct {
            int unused{};
        } card;
        struct {
            int cardCount{};
        } deck;
    } data {};
};
