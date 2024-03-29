#include "sprite.h"
#include "depot.h"

// TODO: Just hard-code spritesheet frame rects in the file.. get rid of this nonsense
void Sprite::UpdateRect(Depot &depot)
{
    if (!spritesheetKey) {
        SDL_LogWarn(0, "Can't update sprite rect when sprite has no spritesheet");
        return;
    }
    if (!animationKey) {
        SDL_LogWarn(0, "Can't update sprite rect when sprite has no animation");
        return;
    }

    const ResourceDB::Spritesheet *sheet = depot.resources->spritesheets()->LookupByKey(spritesheetKey);
    if (!sheet) {
        SDL_LogWarn(0, "Can't update sprite rect when spritesheetKey [%s] is invalid", spritesheetKey);
        return;
    }

    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (position) {
        // NOTE: If you ever change this, you probably also need to make SetAnim*
        // functions call UpdateBbox again (e.g. if frames can be diff sizes)
        position->size.x = sheet->cell_width();
        position->size.y = sheet->cell_height();
    } else {
        SDL_LogWarn(0, "Can't update position size when sprite has no position");
    }

    Texture *texture = depot.renderSystem.FindOrCreateTextureBMP(depot, sheet->texture_key()->c_str());
    if (!texture) {
        SDL_LogError(0, "Can't update sprite rect when spritesheet has no texture");
        return;
    }

    cached_sdl_texture = texture->sdl_texture;

    int cell = -1;
    if (animationKey) {
        const ResourceDB::Animation *anim = sheet->animations()->LookupByKey(animationKey);
        if (anim) {
            cell = anim->frame_start() + frame;
        }
    }

    if (cell < 0) {
        SDL_LogError(0, "Can't update sprite rect for invalid animation name '%s'\n", animationKey);
        return;
    }

    int sheetWidth = 0;
    int sheetHeight = 0;
    SDL_QueryTexture(texture->sdl_texture, 0, 0, &sheetWidth, &sheetHeight);

    srcRect.x = (cell * (int)sheet->cell_width()) % sheetWidth;
    srcRect.y = (cell * (int)sheet->cell_width()) / sheetWidth;
    srcRect.w = (int)sheet->cell_width();
    srcRect.h = (int)sheet->cell_height();
}
