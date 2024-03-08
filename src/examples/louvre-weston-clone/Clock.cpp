#include <LCompositor.h>
#include <LTexture.h>
#include <LLog.h>
#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <cstring>
#include "Clock.h"
#include "Output.h"

Clock::Clock() noexcept
{
    loadedFont = loadFont("Arial");

    if (loadedFont)
    {
        clockTimer = wl_event_loop_add_timer(LCompositor::eventLoop(), &Clock::timerCallback, this);
        wl_event_source_timer_update(clockTimer, 1);
        LLog::debug("[louvre-weston-clone] Created clock timer.");
    }
}

bool Clock::loadFont(const char *fontName)
{
    FcInit();
    FcConfig *config { FcInitLoadConfigAndFonts() };
    FcPattern *pat { FcNameParse((const FcChar8*)fontName) };
    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcPattern* font { FcFontMatch(config, pat, &result) };
    bool success { false };

    if (font)
    {
        FcChar8* file { NULL };
        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
        {
            LLog::debug("Font %s or similar found: %s", fontName, file);

            if (FT_Init_FreeType(&ft) != 0)
            {
                LLog::error("Failed to init FT_Library.");
                goto freeFC;
            }

            if (FT_New_Face(ft, (const char*)file, 0, &face) != 0)
            {
                LLog::error("Failed to init FT_New_Face.");
                FT_Done_FreeType (ft);
            }
            else
                success = true;
        }
    }

freeFC:
    FcPatternDestroy(font);
    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();
    return success;
}

void Clock::updateClockText()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(text, sizeof(text), "%a %b %d, %I:%M %p", timeinfo);
}

void Clock::updateClockTexture()
{
    if (!loadedFont)
        return;

    if (!texture)
        texture = new LTexture();

    const Int32 fontSize { 2 * (32 - 16) };
    updateClockText();

    FT_UInt charIndex;

    Int64 glyph_width;
    Int64 advance;
    Int64 x_off;
    Int64 y_off;

    UChar8 *buffer;
    Int32 bufferWidth { 0 };
    Int32 bufferHeight;

    if (FT_Set_Pixel_Sizes(face, 0, fontSize) != 0)
    {
        LLog::error("Failed to set FT_Face size.");
        FT_Done_Face(face);
        delete texture;
        texture = nullptr;
        return;
    }

    bufferHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

    // Calc min buffer width and height to fit all characters
    char *character { text };
    while (*character != '\0')
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            delete texture;
            texture = nullptr;
            return;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_ADVANCE_ONLY) != 0)
        {
            LLog::error("Failed to load FT_Face advance.");
            delete texture;
            texture = nullptr;
            return;
        }

        bufferWidth += face->glyph->metrics.horiAdvance >> 6;

        character++;
    }

    LLog::debug("Buffer height %d", bufferHeight);

    if (bufferHeight*bufferWidth > 0)
    {
        buffer = (UChar8*)calloc(1, bufferWidth*bufferHeight*4);
    }
    else
    {
        delete texture;
        texture = nullptr;
        return;
    }

    // Draw glyps to buffer
    UInt32 totalAdvanceX { 0 };
    character = text;
    while (*character != '\0')
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            free(buffer);
            delete texture;
            texture = nullptr;
            return;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT) != 0)
        {
            LLog::error("Failed to load FT_Face default.");
            free(buffer);
            delete texture;
            texture = nullptr;
            return;
        }

        glyph_width = face->glyph->metrics.width / 64;
        advance = face->glyph->metrics.horiAdvance / 64;
        x_off = (advance - glyph_width) / 2;
        y_off = face->size->metrics.ascender/64 - face->glyph->metrics.horiBearingY/64;

        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

        for (UInt32 y = 0; y < face->glyph->bitmap.rows; y++)
        {
            for (UInt32 x = 0; x < face->glyph->bitmap.width; x++)
            {
                Int64 buffIndex = ((y + y_off) * bufferWidth + totalAdvanceX + x_off + x)*4;
                buffer[buffIndex] = 48;
                buffer[buffIndex+1] = 48;
                buffer[buffIndex+2] = 48;
                buffer[buffIndex+3] = face->glyph->bitmap.buffer[y*face->glyph->bitmap.width + x];
            }
        }

        totalAdvanceX += advance;
        character++;
    }

    if (!texture->setDataB(LSize(bufferWidth, bufferHeight), bufferWidth*4, DRM_FORMAT_ABGR8888, buffer))
    {
        free(buffer);
        delete texture;
        texture = nullptr;
        return;
    }

    free(buffer);

    for (Output *o : (std::vector<Output*>&)LCompositor::compositor()->outputs())
    {
        o->redrawClock = true;
        o->repaint();
    }
}

Int32 Clock::timerCallback(void *data)
{
    Clock *clock = (Clock*)data;
    clock->updateClockTexture();
    wl_event_source_timer_update(clock->clockTimer, millisecondsUntilNextMinute() + 1500);
    return 0;
}

Int32 Clock::millisecondsUntilNextMinute()
{
    time_t rawtime;
    struct tm *timeinfo;
    struct timespec spec;

    // Get the current time
    clock_gettime(CLOCK_REALTIME, &spec);
    rawtime = spec.tv_sec;
    timeinfo = localtime(&rawtime);

    // Calculate the number of seconds until the next minute
    int secondsUntilNextMinute = 60 - timeinfo->tm_sec;

    // Calculate the number of milliseconds until the next minute
    int msUntilNextMinute = secondsUntilNextMinute * 1000 - spec.tv_nsec / 1000000;

    return msUntilNextMinute;
}
