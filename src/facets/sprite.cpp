#include "sprite.h"
#include "depot.h"

void Sprite::UpdateRect(Depot &depot)
{
    if (!spritesheet) {
        SDL_LogWarn(0, "WARN: Can't update sprite rect when sprite has no spritesheet");
        return;
    }
    if (!animation) {
        SDL_LogWarn(0, "WARN: Can't update sprite rect when sprite has no animation");
        return;
    }

    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (!position) {
        SDL_LogWarn(0, "WARN: Can't update sprite rect when sprite has no position");
        return;
    }

    Spritesheet *sheet = (Spritesheet *)depot.GetFacet(spritesheet, Facet_Spritesheet);
    if (!sheet) {
        SDL_LogWarn(0, "WARN: Can't update sprite rect when sprite has no spritesheet");
        return;
    }

    // NOTE: If you ever change this, you probably also need to make SetAnim*
    // functions call UpdateBbox again (e.g. if frames can be diff sizes)
    position->size = sheet->cellSize;

    ///////////////////////////////////////////////////////////////////////////

    Texture *texture = (Texture *)depot.GetFacet(sheet->texture, Facet_Texture);
    if (!texture) {
        SDL_LogError(0, "Can't update sprite rect when spritesheet has no texture");
        return;
    }

    cached_sdl_texture = texture->sdl_texture;


    int cell = -1;
    if (animation) {
        const auto &result = sheet->animations_by_name.find(animation);
        if (result != sheet->animations_by_name.end()) {
            Animation &anim = sheet->animations[result->second];
            cell = anim.start + frame;
        }
    }

    if (cell < 0) {
        SDL_LogError(0, "Can't update sprite rect for invalid animation name '%s'\n", animation);
        return;
    }

    int sheetWidth = 0;
    int sheetHeight = 0;
    SDL_QueryTexture(texture->sdl_texture, 0, 0, &sheetWidth, &sheetHeight);

    srcRect.x = (cell * (int)sheet->cellSize.x) % sheetWidth;
    srcRect.y = (cell * (int)sheet->cellSize.x) / sheetWidth;
    srcRect.w = (int)sheet->cellSize.x;
    srcRect.h = (int)sheet->cellSize.y;
}
