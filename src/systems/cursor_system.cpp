#include "cursor_system.h"
#include "../facets/depot.h"

void CursorSystem::UpdateCursors(Depot &depot)
{
    for (Cursor &cursor : depot.cursor) {
        Position *cursorPos = (Position *)depot.GetFacet(cursor.uid, Facet_Position);
        DLB_ASSERT(cursorPos);
        if (!cursorPos) {
            printf("WARN: Can't update a cursor with no position");
            continue;
        }

        float x = 0;
        float y = 0;
        uint32_t buttonMask = SDL_GetMouseState(&x, &y);
        cursorPos->pos.x = x;
        cursorPos->pos.y = y;

        cursor.leftButtonQuickClick = false;
        cursor.rightButtonQuickClick = false;

        bool leftButton = buttonMask & SDL_BUTTON_LMASK;
        if (leftButton && !cursor.leftButtonDownAt) {
            cursor.leftButtonDownAt = depot.Now();
        } else if (!leftButton && cursor.leftButtonDownAt) {
            if (depot.Now() - cursor.leftButtonDownAt < cursor.quickClickMaxDt) {
                cursor.leftButtonQuickClick = true;
            }
            cursor.leftButtonDownAt = 0;
        }

        bool rightButton = buttonMask & SDL_BUTTON_RMASK;
        if (rightButton && !cursor.rightButtonDownAt) {
            cursor.rightButtonDownAt = depot.Now();
        } else if (!rightButton && cursor.rightButtonDownAt) {
            if (depot.Now() - cursor.rightButtonDownAt < cursor.quickClickMaxDt) {
                cursor.rightButtonQuickClick = true;
            }
            cursor.rightButtonDownAt = 0;
        }
    }
}

struct DragTarget {
    UID   uid    {};
    vec2  offset {};
};

DragTarget GetDragTarget(Depot &depot, UID cursor, const CollisionList &collisionList)
{
    DragTarget dragTarget{};

    float maxDepth = 0;
    for (const Collision &collision : collisionList) {
        UID   uid    {};
        float depth  {};
        vec2  offset {};

        if (cursor == collision.uidA) {
            uid = collision.uidB;
            depth = collision.bboxB.y + collision.bboxB.h;
            offset = {
                (float)(collision.bboxA.x - collision.bboxB.x),
                (float)(collision.bboxA.y - collision.bboxB.y)
            };
        } else if (cursor == collision.uidB) {
            uid = collision.uidA;
            depth = collision.bboxA.y + collision.bboxA.h;
            offset = {
                (float)(collision.bboxB.x - collision.bboxA.x),
                (float)(collision.bboxB.y - collision.bboxA.y)
            };
        } else {
            continue;
        }

        if (depth < maxDepth) {
            continue;
        }

        Card *card = (Card *)depot.GetFacet(uid, Facet_Card);
        if (card) {
            // Disallow clicking invulnerable cards
            if (card && card->noClickUntil > depot.Now()) {
                continue;
            }

            dragTarget.uid = uid;
            dragTarget.offset = offset;
            maxDepth = depth;
        }
    }

    return dragTarget;
}

void CursorSystem::UpdateDragTargets(Depot &depot, const CollisionList &collisionList)
{
    for (Cursor &cursor : depot.cursor) {
        Position *cursorPos = (Position *)depot.GetFacet(cursor.uid, Facet_Position);
        DLB_ASSERT(cursorPos);
        if (!cursorPos) {
            printf("WARN: Can't update a cursor with no position");
            continue;
        }

        vec2 currentPos{
            cursorPos->pos.x,
            cursorPos->pos.y
        };

        bool endDrag = false;

        // Update uid drag subject
        if (cursor.leftButtonDownAt) {
            // Check if there's something to start dragging
            if (!cursor.uidDragSubject) {
                DragTarget dragTarget = GetDragTarget(depot, cursor.uid, collisionList);
                if (dragTarget.uid) {
                    cursor.dragBeginPos = currentPos;
                    cursor.uidDragSubject = dragTarget.uid;
                    cursor.dragSubjectOffset = dragTarget.offset;

                    vec2 dragOffset{};
                    dragOffset.x = currentPos.x - cursor.dragBeginPos.x;
                    dragOffset.y = currentPos.y - cursor.dragBeginPos.y;

                    Message dragBegin{};
                    dragBegin.type = MsgType_Cursor_Notify_DragBegin;
                    dragBegin.uid = cursor.uidDragSubject;
                    dragBegin.data.cursor_dragevent.startPos = cursor.dragBeginPos;
                    dragBegin.data.cursor_dragevent.currentPos = currentPos;
                    dragBegin.data.cursor_dragevent.dragOffset = dragOffset;
                    dragBegin.data.cursor_dragevent.subjectOffset = cursor.dragSubjectOffset;
                    depot.msgQueue.push_back(dragBegin);
                }
            }
        } else if (cursor.uidDragSubject) {
            vec2 dragOffset{};
            dragOffset.x = currentPos.x - cursor.dragBeginPos.x;
            dragOffset.y = currentPos.y - cursor.dragBeginPos.y;

            Message dragEnd{};
            dragEnd.type = MsgType_Cursor_Notify_DragEnd;
            dragEnd.uid = cursor.uidDragSubject;
            dragEnd.data.cursor_dragevent.startPos = cursor.dragBeginPos;
            dragEnd.data.cursor_dragevent.currentPos = currentPos;
            dragEnd.data.cursor_dragevent.dragOffset = dragOffset;
            dragEnd.data.cursor_dragevent.subjectOffset = cursor.dragSubjectOffset;
            depot.msgQueue.push_back(dragEnd);

            // Button no longer held down, end drag
            endDrag = true;
        }

        // Update drag subject's position
        if (cursor.uidDragSubject) {
            Position *subjectPos = (Position *)depot.GetFacet(cursor.uidDragSubject, Facet_Position);
            if (subjectPos) {
                vec2 dragOffset{};
                dragOffset.x = currentPos.x - cursor.dragBeginPos.x;
                dragOffset.y = currentPos.y - cursor.dragBeginPos.y;

                Message dragUpdate{};
                dragUpdate.type = MsgType_Cursor_Notify_DragUpdate;
                dragUpdate.uid = cursor.uidDragSubject;
                dragUpdate.data.cursor_dragevent.startPos = cursor.dragBeginPos;
                dragUpdate.data.cursor_dragevent.currentPos = currentPos;
                dragUpdate.data.cursor_dragevent.dragOffset = dragOffset;
                dragUpdate.data.cursor_dragevent.subjectOffset = cursor.dragSubjectOffset;
                depot.msgQueue.push_back(dragUpdate);
            } else {
                // Whatever we were dragging has disappeared, end drag
                SDL_LogError(0, "Drag subject position vanished %u", cursor.uidDragSubject);
                endDrag = true;
            }
        }

        if (endDrag) {
            cursor.dragBeginPos = {};
            cursor.uidDragSubject = 0;
            cursor.dragSubjectOffset = {};
        }
    }
}
