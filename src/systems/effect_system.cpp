#include "effect_system.h"
#include "../facets/depot.h"

void ApplyEffectToMaterial(Depot &depot, Material &material, Effect &effect)
{
    MaterialProto *materialProto = (MaterialProto *)depot.GetFacet(material.materialProto, Facet_MaterialProto);
    if (!materialProto) {
        DLB_ASSERT(!"dafuq");
        return;
    }

    switch (effect.type) {
        case Effect_IgniteFlammable: {
            if (materialProto->flags.test(MaterialFlag_Flammable) && !material.state.test(MaterialState_OnFire)) {
                material.state.set(MaterialState_OnFire);

                Message onFireBegin{};
                onFireBegin.type = MsgType_Effect_OnFireBegin;
                onFireBegin.uid = material.uid;
                depot.msgQueue.push_back(onFireBegin);
            }
            break;
        }
        case Effect_ExtinguishFlammable: {
            if (materialProto->flags.test(MaterialFlag_Flammable) && material.state.test(MaterialState_OnFire)) {
                material.state.reset(MaterialState_OnFire);

                Message onFireEnd{};
                onFireEnd.type = MsgType_Effect_OnFireEnd;
                onFireEnd.uid = material.uid;
                depot.msgQueue.push_back(onFireEnd);
            }
            break;
        }
    }
}

void ApplyEffectsToMaterial(Depot &depot, Material &material, EffectList &effectList)
{
    for (Effect &effect : effectList.effects) {
        ApplyEffectToMaterial(depot, material, effect);
    }
}

void EffectSystem::ApplyDragFx(Depot &depot, const CollisionList &collisionList)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];

        switch (msg.type) {
            case MsgType_Card_Notify_DragUpdate: {
                UID applyFxUid = msg.uid;
                for (const Collision &collision : collisionList) {
                    UID recvFxUid = 0;
                    if (collision.uidA == 26 && collision.uidB == 29) {
                        printf("");
                    }

                    if (applyFxUid == collision.uidA) {
                        recvFxUid = collision.uidB;
                    } else if (applyFxUid == collision.uidB) {
                        recvFxUid = collision.uidA;
                    }

                    Card *card = (Card *)depot.GetFacet(applyFxUid, Facet_Card);
                    if (!card) {
                        continue;
                    }

                    CardProto *cardProto = (CardProto *)depot.GetFacet(card->cardProto, Facet_CardProto);
                    if (!cardProto) {
                        DLB_ASSERT(!"Huh? Card without proto??");
                        continue;
                    }

                    EffectList *effectList = (EffectList *)depot.GetFacet(cardProto->effectList, Facet_EffectList);
                    Material *material = (Material *)depot.GetFacet(recvFxUid, Facet_Material);
                    if (material && effectList) {
                        ApplyEffectsToMaterial(depot, *material, *effectList);
                        break;
                    }
                }
                break;
            }
            default: break;
        }
    }
}

void EffectSystem::ApplyFx_AnyToAny(Depot &depot, const CollisionList &collisionList)
{
    for (const Collision &collision : collisionList) {
        if (collision.uidA == 26 && collision.uidB == 29) {
            printf("");
        }

        Card *aCard = (Card *)depot.GetFacet(collision.uidA, Facet_Card);
        Card *bCard = (Card *)depot.GetFacet(collision.uidB, Facet_Card);
        if (!aCard || !bCard) {
            continue;
        }

        CardProto *aCardProto = (CardProto *)depot.GetFacet(aCard->cardProto, Facet_CardProto);
        CardProto *bCardProto = (CardProto *)depot.GetFacet(bCard->cardProto, Facet_CardProto);
        if (!aCardProto || !bCardProto) {
            DLB_ASSERT(!"Huh? Card without proto??");
            continue;
        }

        EffectList *aFx = (EffectList *)depot.GetFacet(aCardProto->effectList, Facet_EffectList);
        EffectList *bFx = (EffectList *)depot.GetFacet(bCardProto->effectList, Facet_EffectList);

        Material *aMat = (Material *)depot.GetFacet(collision.uidA, Facet_Material);
        Material *bMat = (Material *)depot.GetFacet(collision.uidB, Facet_Material);

        if (aMat && bFx) {
            ApplyEffectsToMaterial(depot, *aMat, *bFx);
        }
        if (bMat && aFx) {
            ApplyEffectsToMaterial(depot, *bMat, *aFx);
        }
    }
}
