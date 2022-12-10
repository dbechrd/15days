#include "cursor_system.h"
#include "../facets/depot.h"

void CursorSystem::Update(Depot &depot)
{
    for (Cursor &cursor : depot.cursor) {
        Position *position = (Position *)depot.GetFacet(cursor.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't update a cursor with no position");
            continue;
        }

        int x = 0;
        int y = 0;
        SDL_GetMouseState(&x, &y);
        position->pos.x = (float)x;
        position->pos.y = (float)y;

        if (cursor.uidDragSubject) {
            Position *subjectPos = (Position *)depot.GetFacet(cursor.uidDragSubject, Facet_Position);
            if (!subjectPos) {
                // Whatever we were dragging has disappeared, reset drag state
                cursor.uidDragSubject = 0;
                cursor.dragOffset = {};
            }
        }
    }
}
