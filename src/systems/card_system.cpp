#include "card_system.h"
#include "../facets/depot.h"

void CardSystem::UpdateCards(Depot &depot)
{
    for (Card &card : depot.card) {
        Sprite *sprite = (Sprite *)depot.GetFacet(card.uid, Facet_Sprite);
        if (!sprite) {
            printf("WARN: Can't update a card with no sprite");
            continue;
        }

        CardProto *cardProto = (CardProto *)depot.GetFacet(card.cardProto, Facet_CardProto);
        if (!cardProto) {
            printf("WARN: Can't update a card with no card prototype");
            continue;
        }

        if (card.noClickUntil > depot.Now()) {
            sprite->SetAnimIndex(depot, 2);  // TODO: Clean up magic number, move deck back to 0
            //sprite->color = C255(COLOR_GRAY_5);
        } else {
            sprite->SetAnimIndex(depot, cardProto->animation);
            //sprite->color = {};
            card.noClickUntil = 0;
        }
    }
}