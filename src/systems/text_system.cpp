#include "text_system.h"
#include "../facets/depot.h"

UID TextSystem::CreateNarrator(Depot &depot)
{
    UID uidNarrator = depot.Alloc("narrator");

    Position *position = (Position *)depot.AddFacet(uidNarrator, Facet_Position);
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(depot.renderSystem.Window(), &windowWidth, &windowHeight);
    position->pos.x = windowWidth / 2.0f;
    position->pos.y = 200.0f;

    if (depot.card.size()) {
        Position *campPos = (Position *)depot.GetFacet(depot.card.front().uid, Facet_Position);
        if (campPos) {
            position->pos.x = campPos->pos.x - 100.0f;
            position->pos.y = campPos->pos.y - 100.0f;
        }
    }

    Text *text = (Text *)depot.AddFacet(uidNarrator, Facet_Text);
#if 0
    text->font = depot.textSystem.LoadFont(depot, "font/KarminaBold.otf", 64);
    text->str = "15 Days";
#endif
#if 1
    position->pos.x = 10.0f;
    position->pos.y = 4.0f;
    text->fontKey = "karmina_bold_64";
    text->str =
        C_RED     "Red"
        C_GREEN   " Green"
        C_BLUE    " Blue"
        C_CYAN    " Cyan"
        C_MAGENTA " Magenta"
        C_YELLOW  " Yellow"
        C_WHITE   " White";
#endif
#if 0
    text->font = depot.textSystem.LoadFont(depot, "font/OpenSans-Bold.ttf", 20);
    text->str = "The`g camp`w is your home.\n"
        "Your adventure starts here.\n"
        "`r+10 health`w while in camp.";
#endif

    text->align = TextAlign_VBottom_HCenter;

    // TODO: NarratorSystem
    // - Check if position.pos + sprite.size outside of screen w/h
    //       Msg_Physics_Notify_Collide (uid=player)
    //         v
    //       NarrationEvent_LeaveScreen (uid=player)
    //         v
    //       Msg_Narrator_Says (uid=narrator, text="collided!")
    // - NarratorSystem::Update()
    //   - If delay timer elapsed, dequeue oldest item in the narrationQueue
    // - NarratorSystem::Draw(narratorQueue, drawList);
    //   - Generate draw commands for active text using the narrationQueue

    return uidNarrator;
}

Font *TextSystem::FindOrLoadFont(Depot &depot, const char *fontKey)
{
    // Check if already loaded
    Font *existingFont = (Font *)depot.GetFacetByName(fontKey, Facet_Font);
    if (existingFont) {
        return existingFont;
    }

    const char *ttf_path = 0;
    int point_size = 0;

    const ResourceDB::Font *dbFont = depot.resources->fonts()->LookupByKey(fontKey);
    if (dbFont) {
        ttf_path = dbFont->ttf_path()->c_str();
        point_size = dbFont->point_size();
    } else {
        SDL_LogError(0, "Invalid font key: %s", fontKey);
    }

    TTF_Font *ttfFont = TTF_OpenFont(ttf_path, point_size);
    if (!ttfFont) {
        SDL_LogError(0, "Failed to load font %s\n  %s\n", ttf_path, TTF_GetError());
        return 0;
    }

    UID uidFont = depot.Alloc(fontKey);
    Font *font = (Font *)depot.AddFacet(uidFont, Facet_Font);
    font->fontKey = fontKey;
#if 0
    font->outline = 0;
    font->outlineOffset = { 2, 1 };
    if (ptsize >= 32) {
        font->outlineOffset.x += 1;
        font->outlineOffset.y += 1;
    }
#else
    font->outline = 1;
    font->outlineOffset = { 0 };
#endif
    font->ttf_font = ttfFont;
    return font;
}

void TextSystem::Init(Depot &depot)
{
    uidNarrator = CreateNarrator(depot);
}

void TextSystem::PushUpdateText(Depot &depot, UID uidText, vec2 offset, vec4 color,
    const char *str, bool strOwner)
{
    Text *text = (Text *)depot.GetFacet(uidText, Facet_Text);
    if (!text) return;

    if (text->strOwner) {
        free((void *)text->str);
        text->strOwner = false;
    }

    text->str = str;
    text->strOwner = strOwner;
    text->offset = offset;
}

void TextSystem::PushUpdateNarrator(Depot &depot, vec2 offset, vec4 color,
    const char *str, double displayFor, bool interrupt)
{
    if (interrupt) {
        updateNarratorQueue.clear();
        narratorMsgStartedAt = 0;
    }

    size_t len = strlen(str);
    char *copyOfStr = 0;
    if (len) {
        copyOfStr = (char *)calloc(len + 1, 1);
        strncpy(copyOfStr, str, len);
    }

    Text_UpdateNarratorRequest updateNarratorRequest{};
    updateNarratorRequest.str = copyOfStr;
    updateNarratorRequest.offset = offset;
    updateNarratorRequest.color = color;
    updateNarratorRequest.displayFor = displayFor;
    updateNarratorQueue.push_back(updateNarratorRequest);
}

void TextSystem::Update(Depot &depot)
{
    if (!updateNarratorQueue.size()) return;

    // Get current message
    const Text_UpdateNarratorRequest *msg = &updateNarratorQueue.front();
    const Text_UpdateNarratorRequest *nextMsg = 0;

    if (!narratorMsgStartedAt) {
        // Narrator idle, start displaying the message
        nextMsg = msg;
    } else if (depot.Now() - narratorMsgStartedAt >= msg->displayFor) {
        // Narrator displaying expired message, show next message
        updateNarratorQueue.pop_front();
        nextMsg = updateNarratorQueue.size() ? &updateNarratorQueue.front() : 0;
        if (!nextMsg) {
            // If no next message, clear the message
            static Text_UpdateNarratorRequest emptyMsg{ "", {}, {}, 0 };
            nextMsg = &emptyMsg;
        }
    }

    if (nextMsg) {
        PushUpdateText(depot, uidNarrator, nextMsg->offset, nextMsg->color,
            nextMsg->str, true);
        narratorMsgStartedAt = nextMsg->str ? depot.Now() : 0;
    }
}

void TextSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    static bool debugKern = false;

    for (Text &text : depot.text) {
        if (!text.str) {
            continue;
        }

        Position *position = (Position *)depot.GetFacet(text.uid, Facet_Position);
        if (!position) {
            printf("WARN: Can't draw text with no position\n");
            continue;
        }

        float x = position->pos.x + text.offset.x;
        float y = position->pos.y - position->pos.z + text.offset.y;

        Font *font = FindOrLoadFont(depot, text.fontKey);
        if (!font) {
            printf("WARN: Can't draw text with no font\n");
            continue;
        }

        bool hasKerning = TTF_GetFontKerning(font->ttf_font);

        // TODO: TextAlign (always centered along x and y for now)

        const char *c = text.str;
        vec2 cursor{ x, y };
        float lineHeight = 0;

        vec4 color = C255(COLOR_WHITE);

        char cPrev = 0;
        while (*c) {
            switch (*c) {
                case '\n': {
                    cursor.x = x;
                    cursor.y += lineHeight;
                    c++;
                    continue;
                }
            }

            char cNext = *(c+1);
            // `` = literal backtick, don't do this color stuffs
            if (*c == '`' && cNext != '`') {
                c++;
                DLB_ASSERT(*c);
                switch (*c) {
                    case C_WHITE[1]:   color = { 255, 255, 255, 255 }; c++; break;
                    case C_RED[1]:     color = { 165,   0,   0, 255 }; c++; break;
                    case C_GREEN[1]:   color = {  49, 165,   0, 255 }; c++; break;
                    case C_BLUE[1]:    color = {  43,  75, 255, 255 }; c++; break;
                    case C_CYAN[1]:    color = {   0, 196, 196, 255 }; c++; break;
                    case C_MAGENTA[1]: color = { 198,  79, 198, 255 }; c++; break;
                    case C_YELLOW[1]:  color = { 234, 199,   0, 255 }; c++; break;
                    case C_BLACK[1]:   color = {   0,   0,   0, 255 }; c++; break;
                    default: printf("WARN: Dats a weird color value, mang\n"); break;
                }
                continue;
            }

            if (!font->glyphCache.rects.contains((uint32_t)*c)) {
                c++;
                continue;
            }

            if (hasKerning) {
                int kern = TTF_GetFontKerningSizeGlyphs32(font->ttf_font, cPrev, *c);
                if (debugKern) {
                    printf("Kern for '%c','%c' = %d\n", cPrev, *c, kern);
                }
                cursor.x += (float)kern;
            }

            rect glyphRect = font->glyphCache.rects[*c];
            DLB_ASSERT(glyphRect.w);
            DLB_ASSERT(glyphRect.h);
            DLB_ASSERT(font->glyphCache.atlasTexture);

            rect drawRect = glyphRect;
            drawRect.x = cursor.x;
            drawRect.y = cursor.y;

            DrawCommand drawGlyph{};
            drawGlyph.uid = text.uid;
            drawGlyph.srcRect = glyphRect;
            drawGlyph.texture = font->glyphCache.atlasTexture;
            drawGlyph.dstRect = drawRect;
            drawGlyph.color = color;
            drawGlyph.depth = 1;
            drawQueue.push_back(drawGlyph);

            cursor.x += glyphRect.w - font->outline;
            lineHeight = MAX(lineHeight, glyphRect.h);
            cPrev = *c;
            c++;
        }
    }

    debugKern = false;
}