#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <LLog.h>
#include <LSize.h>
#include <LTexture.h>
#include "TextRenderer.h"

TextRenderer::TextRenderer(const char *fontName)
{
    FcInit();
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pat = FcNameParse((const FcChar8*)fontName);
    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcPattern* font = FcFontMatch(config, pat, &result);
    loadedFont = false;

    if (font)
    {
        FcChar8* file = NULL;
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
                FT_Done_FreeType(ft);
            }
            else
                loadedFont = true;
        }
    }

freeFC:
    FcPatternDestroy(font);
    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();
}

TextRenderer *TextRenderer::loadFont(const char *fontName)
{
    TextRenderer *renderer = new TextRenderer(fontName);

    if (renderer->loadedFont)
        return renderer;

    delete renderer;
    return nullptr;
}

TextRenderer::~TextRenderer()
{
    if (loadedFont)
    {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
}

LTexture *TextRenderer::renderText(const char *text, Int32 fontSize, UChar8 r, UChar8 g, UChar8 b)
{
    LSize bufferSize = calculateTextureSize(text, fontSize);

    if (bufferSize.area() == 0)
        return nullptr;

    FT_UInt charIndex;

    Int64 glyph_width;
    Int64 advance;
    Int64 x_off;
    Int64 y_off;

    UChar8 *buffer = (UChar8*)calloc(1, bufferSize.area()*4);

    // Draw glyps to buffer
    UInt32 totalAdvanceX = 0;
    const char *character = text;

    while (*character != '\0')
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            free(buffer);
            return nullptr;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT) != 0)
        {
            LLog::error("Failed to load FT_Face default.");
            free(buffer);
            return nullptr;
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
                Int64 buffIndex = ((y + y_off) * bufferSize.width() + totalAdvanceX + x_off + x)*4;
                buffer[buffIndex] = r;
                buffer[buffIndex+1] = g;
                buffer[buffIndex+2] = b;
                buffer[buffIndex+3] = face->glyph->bitmap.buffer[y*face->glyph->bitmap.width + x];
            }
        }

        totalAdvanceX += advance;
        character++;
    }

    LTexture *texture = new LTexture();

    if (!texture->setDataB(bufferSize, bufferSize.w()*4, DRM_FORMAT_ABGR8888, buffer))
    {
        free(buffer);
        delete texture;
        return nullptr;
    }

    free(buffer);

    return texture;
}

LSize TextRenderer::calculateTextureSize(const char *text, Int32 fontSize)
{
    LSize bufferSize = LSize(0,0);
    FT_UInt charIndex;

    if (FT_Set_Pixel_Sizes(face, 0, fontSize) != 0)
    {
        LLog::error("Failed to set FT_Face size.");
        FT_Done_Face(face);
        return bufferSize;
    }

    bufferSize.setH((face->size->metrics.ascender - face->size->metrics.descender) >> 6);

    // Calc min buffer width and height to fit all characters
    const char *character = text;
    while (*character != '\0')
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            return bufferSize;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_ADVANCE_ONLY) != 0)
        {
            LLog::error("Failed to load FT_Face advance.");
            return bufferSize;
        }

        bufferSize.setW(bufferSize.w() + (face->glyph->metrics.horiAdvance >> 6));

        character++;
    }

    return bufferSize;
}
