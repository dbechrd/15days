#include "effect_system.h"
#include "../facets/depot.h"

void ApplyEffectsToMaterial(Depot &depot, Material &material, const char *elementProtoKey)
{
    const ResourceDB::MaterialProto *materialProto = depot.resources->material_protos()->LookupByKey(material.materialProtoKey);
    if (!materialProto) {
        SDL_LogError(0, "Material has invalid material prototype %s\n", material.materialProtoKey);
        return;
    }
    const ResourceDB::MaterialAttribs &matAttribs = materialProto->attribs();

    const ResourceDB::ElementProto *elementProto = depot.resources->element_protos()->LookupByKey(elementProtoKey);
    if (!elementProto) {
        SDL_LogError(0, "Invalid element prototype key %s\n", elementProtoKey);
        return;
    }

    const ResourceDB::MaterialAttribs affectedMaterialsMask = elementProto->affected_materials_mask();

    const ResourceDB::MaterialStates affectedStatesMask = elementProto->affected_states_mask();
    const ResourceDB::MaterialStates pendingNewStates = elementProto->new_states();
    //const ResourceDB::MaterialState effectiveNewState =
    //    (ResourceDB::MaterialState)(newState & affectedStateMask);

    if ((matAttribs & affectedMaterialsMask) > 0) {
        ResourceDB::MaterialStates oldStates = material.states;
        ResourceDB::MaterialStates newStates = (ResourceDB::MaterialStates)(
            (material.states & ~affectedStatesMask) |
            (pendingNewStates & affectedStatesMask)
        );
        ResourceDB::MaterialStates deltaStates = (ResourceDB::MaterialStates)(
            oldStates ^ newStates
        );

        if (deltaStates > 0) {
            material.states = newStates;
            Message msgStateChange{};
            msgStateChange.type = MsgType_Material_StateChange;
            msgStateChange.uid = material.uid;
            msgStateChange.data.material_statechange.oldStates = oldStates;
            msgStateChange.data.material_statechange.newStates = newStates;
            msgStateChange.data.material_statechange.deltaStates = deltaStates;
            depot.msgQueue.push_back(msgStateChange);
        }
    }

#if 0
    // Effect types
    const bool fxIgniteFlammable = elemAttribs & ResourceDB::ElementAttribs_IgniteFlammable;
    const bool fxExtinguishFlammable = elemAttribs & ResourceDB::ElementAttribs_ExtinguishFlammable;

    // Material attributes
    const bool isFlammable = matAttribs & ResourceDB::MaterialAttribs_Flammable;

    // Ignite flammable materials
    if (fxIgniteFlammable && isFlammable && !material.state.test(MaterialState_OnFire)) {
        material.state.set(MaterialState_OnFire);

        Message onFireBegin{};
        onFireBegin.type = MsgType_Effect_OnFireBegin;
        onFireBegin.uid = material.uid;
        depot.msgQueue.push_back(onFireBegin);
    }

    // Extinguish flammable materials
    if (fxExtinguishFlammable && isFlammable && material.state.test(MaterialState_OnFire)) {
        material.state.reset(MaterialState_OnFire);

        Message onFireEnd{};
        onFireEnd.type = MsgType_Effect_OnFireEnd;
        onFireEnd.uid = material.uid;
        depot.msgQueue.push_back(onFireEnd);
    }
#endif
}

void EffectSystem::ApplyDragFx(Depot &depot, const CollisionList &collisionList)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];

        switch (msg.type) {
            case MsgType_Cursor_Notify_DragUpdate: {
                UID applyFxUid = msg.uid;
                for (const Collision &collision : collisionList) {
                    UID recvFxUid = 0;
                    if (applyFxUid == collision.uidA) {
                        recvFxUid = collision.uidB;
                    } else if (applyFxUid == collision.uidB) {
                        recvFxUid = collision.uidA;
                    } else {
                        continue;
                    }

                    Card *card = (Card *)depot.GetFacet(applyFxUid, Facet_Card);
                    if (!card) {
                        continue;
                    }

                    const ResourceDB::CardProto *cardProto =
                        depot.resources->card_protos()->LookupByKey(card->cardProto);
                    if (!cardProto) {
                        printf("WARN: Can't update a card with no card prototype");
                        continue;
                    }

                    Material *material = (Material *)depot.GetFacet(recvFxUid, Facet_Material);
                    if (material && cardProto->element_proto()) {
                        ApplyEffectsToMaterial(depot, *material, cardProto->element_proto()->c_str());
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
#if 0
    for (const Collision &collision : collisionList) {
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
#else
    DLB_ASSERT(!"nope, old code");
#endif
}