#include "card_system.h"
#include "../facets/depot.h"

void CardSystem::React(double now, Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = depot.msgQueue[i];
        Cursor *cursor = (Cursor *)depot.GetFacet(msg.uid, Facet_Cursor);
        if (!cursor) {
            continue;
        }

        // This seems like it should generate Card_DragBegin.. not receive it..
        // Perhaps there's a missing layer of Msg_Input_DragRequest or something?
        switch (msg.type) {
            case MsgType_Global_PrimaryPress: {
                int x = 0;
                int y = 0;
                SDL_GetMouseState(&x, &y);
                cursor->uidDragSubject = FindCardAtScreenPos(depot, x, y, &cursor->dragOffset);
                if (cursor->uidDragSubject) {
                    Message dragBegin{};
                    dragBegin.type = MsgType_Card_DragBegin;
                    dragBegin.uid = cursor->uidDragSubject;
                    depot.msgQueue.push_back(dragBegin);
                }
                break;
            }
            case MsgType_Global_PrimaryRelease: {
                if (cursor->uidDragSubject) {
                    Message dragEnd{};
                    dragEnd.type = MsgType_Card_DragEnd;
                    dragEnd.uid = cursor->uidDragSubject;
                    depot.msgQueue.push_back(dragEnd);

                    cursor->uidDragSubject = 0;
                }
                break;
            }
            default: break;
        }
    }
}

// TODO: CursorSystem instead of CardSystem!?!?
void CardSystem::Behave(double now, Depot &depot, double dt)
{
    for (Cursor &cursor : depot.cursor) {
        SDL_GetMouseState(&cursor.posScreen.x, &cursor.posScreen.y);
        Position *position = (Position *)depot.GetFacet(cursor.uidDragSubject, Facet_Position);
        if (position) {
            position->pos.x = (float)cursor.posScreen.x - cursor.dragOffset.x;
            position->pos.y = (float)cursor.posScreen.y - cursor.dragOffset.y;
        }
    }
}

UID CardSystem::FindCardAtScreenPos(Depot &depot, int x, int y, vec2 *offset)
{
    for (Position &position : depot.position) {
        vec2 pos = { position.pos.x, position.pos.y - position.pos.z };
        vec2 size = { 20, 20 };

        // TODO: If entity has Sprite *and* Text, check *both* bounding boxes
        // and allow drag if either is clicked
        Sprite *sprite = (Sprite *)depot.GetFacet(position.uid, Facet_Sprite);
        if (sprite) {
            size = sprite->size;
        } else {
            // TODO: Have to account for both the text offset (in pos check)
            // as well as alignment offsets for this to work correctly. We
            // should make Sprite->GetBBox() and Text->GetBBox() helpers or
            // something.
            Text *text = (Text *)depot.GetFacet(position.uid, Facet_Text);
            if (text) {
                Texture *texture = (Texture *)depot.GetFacet(position.uid, Facet_Texture);
                if (texture) {
                    size = texture->size;
                }
            }
        }

        if (x >= pos.x && x < pos.x + size.x && y >= pos.y && y < pos.y + size.y) {
            if (offset) {
                *offset = { x - pos.x, y - pos.y };
            }
            return position.uid;
        }
    }
    return 0;
}