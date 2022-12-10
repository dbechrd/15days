#include "card_system.h"
#include "../facets/depot.h"

void CardSystem::React(double now, Depot &depot)
{
    /*size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        Cursor *cursor = (Cursor *)depot.GetFacet(msg.uid, Facet_Cursor);
        if (!cursor) {
            continue;
        }

        switch (msg.type) {
            case MsgType_Cursor_PrimaryPress:
            {
                cursor->uidDragSubject = FindCardAtScreenPos(depot, cursor->uid, &cursor->dragOffset);
                if (cursor->uidDragSubject) {
                    Message dragBegin{};
                    dragBegin.type = MsgType_Card_Notify_DragBegin;
                    dragBegin.uid = cursor->uidDragSubject;
                    depot.msgQueue.push_back(dragBegin);
                }
                break;
            }
            case MsgType_Cursor_PrimaryRelease:
            {
                if (cursor->uidDragSubject) {
                    UID landedOn = FindCardAtScreenPos(depot, cursor->uidDragSubject, &cursor->dragOffset);

                    Message dragEnd{};
                    dragEnd.type = MsgType_Card_Notify_DragEnd;
                    dragEnd.uid = cursor->uidDragSubject;
                    dragEnd.data.card_dragend.landedOn = landedOn;
                    depot.msgQueue.push_back(dragEnd);

                    cursor->uidDragSubject = 0;
                }
                break;
            }
            default: break;
        }
    }*/

    for (Cursor &cursor : depot.cursor) {
        if (!cursor.uidDragSubject) {
            continue;
        }

        Position *position = (Position *)depot.GetFacet(cursor.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't use a cursor with no position");
            continue;
        }

        // TODO: This should probably generate a message instead.. like ApplyImpulse
        Position *subjectPos = (Position *)depot.GetFacet(cursor.uidDragSubject, Facet_Position);
        if (subjectPos) {
            subjectPos->pos.x = (float)position->pos.x - cursor.dragOffset.x;
            subjectPos->pos.y = (float)position->pos.y - cursor.dragOffset.y;
        }
    }
}

void CardSystem::Behave(double now, Depot &depot, double dt)
{
}
