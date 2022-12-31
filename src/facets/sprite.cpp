#include "sprite.h"
#include "depot.h"

void Sprite::SetSpritesheet(Depot &depot, UID uidSpritesheet)
{
    spritesheet = uidSpritesheet;
    UpdateRect(depot);
}

void Sprite::SetAnimIndex(Depot &depot, int animIdx) {
    // TODO: Bounds check animation index by looking at spritesheet??
    animation = animIdx;
    UpdateRect(depot);
}

void Sprite::SetAnimFrame(Depot &depot, int animFrame) {
    // TODO: Bounds check animation index by looking at spritesheet??
    frame = animFrame;
    UpdateRect(depot);
}

void Sprite::UpdateRect(Depot &depot)
{
    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (!position) {
        printf("WARN: Can't update sprite rect when sprite has no position");
        return;
    }

    Spritesheet *sheet = (Spritesheet *)depot.GetFacet(spritesheet, Facet_Spritesheet);
    if (!sheet) {
        printf("WARN: Can't update sprite rect when sprite has no spritesheet");
        return;
    }

    // NOTE: If you ever change this, you probably also need to make SetAnim*
    // functions call UpdateBbox again (e.g. if frames can be diff sizes)
    position->size = sheet->cellSize;

    ///////////////////////////////////////////////////////////////////////////

    Texture *texture = (Texture *)depot.GetFacet(sheet->uid, Facet_Texture);
    if (!texture) {
        SDL_LogError(0, "ERROR: Can't update sprite rect when spritesheet has no texture");
        return;
    }

    cached_sdl_texture = texture->sdl_texture;

    Animation &animation = sheet->animations[GetAnimIndex()];
    int cell = animation.start + GetAnimFrame();

    int sheetWidth = 0;
    int sheetHeight = 0;
    SDL_QueryTexture(texture->sdl_texture, 0, 0, &sheetWidth, &sheetHeight);

    srcRect.x = (cell * (int)sheet->cellSize.x) % sheetWidth;
    srcRect.y = (cell * (int)sheet->cellSize.x) / sheetWidth;
    srcRect.w = (int)sheet->cellSize.x;
    srcRect.h = (int)sheet->cellSize.y;
}
