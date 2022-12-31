#include "sprite.h"
#include "depot.h"

void Sprite::SetSpritesheet(Depot &depot, UID uidSpritesheet)
{
    spritesheet = uidSpritesheet;
    UpdateBbox(depot);
}

void Sprite::SetAnimIndex(Depot &depot, int animIdx) {
    // TODO: Bounds check animation index by looking at spritesheet??
    animation = animIdx;

    // NOTE: This can never change size because it's just using sheet->cellSize
    //UpdateBbox(depot);
}

void Sprite::SetAnimFrame(Depot &depot, int animFrame) {
    // TODO: Bounds check animation index by looking at spritesheet??
    frame = animFrame;

    // NOTE: This can never change size because it's just using sheet->cellSize
    //UpdateBbox(depot);
}

void Sprite::UpdateBbox(Depot &depot)
{
    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (!position) {
        printf("WARN: Can't update sprite bbox when sprite has no position");
        return;
    }

    Spritesheet *sheet = (Spritesheet *)depot.GetFacet(spritesheet, Facet_Spritesheet);
    if (!sheet) {
        printf("WARN: Can't update sprite bbox when sprite has no spritesheet");
        return;
    }

    // NOTE: If you ever change this, you probably also need to make SetAnim*
    // functions call UpdateBbox again (e.g. if frames can be diff sizes)
    position->size = sheet->cellSize;
}