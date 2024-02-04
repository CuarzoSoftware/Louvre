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

LTexture *TextRenderer::renderText(const char *text, Int32 fontSize, Int32 maxWidth, UChar8 r, UChar8 g, UChar8 b)
{
    char *clippedText = nullptr;

    LSize bufferSize = calculateTextureSize(text, fontSize);

    if (bufferSize.area() <= 0)
        return nullptr;

    if (maxWidth != -1 && bufferSize.w() > maxWidth)
    {
        Float32 textLen = strlen(text);
        Int32 newLen =  Float32(maxWidth * textLen) / Float32(bufferSize.w()) - 4.f;

        if (newLen < 4)
            return nullptr;

        clippedText = new char[newLen + 4];

        memcpy((void*)clippedText, text, newLen);
        clippedText[newLen + 0] = '.';
        clippedText[newLen + 1] = '.';
        clippedText[newLen + 2] = '.';
        clippedText[newLen + 3] = '\0';

        bufferSize = calculateTextureSize(clippedText, fontSize);
    }
    else
    {
        clippedText = (char*)text;
    }

    bufferSize.setW(bufferSize.w() + (bufferSize.w() % 2));
    bufferSize.setH(bufferSize.h() + (bufferSize.h() % 2));

    UChar32 *utf32 = toUTF32(clippedText);
    UChar32 *character = utf32;

    if (!character)
    {
        if (clippedText != text)
            delete[] clippedText;

        return nullptr;
    }

    if (bufferSize.area() <= 0)
    {
        if (clippedText != text)
            delete[] clippedText;

        if (character)
            free(utf32);

        return nullptr;
    }

    FT_UInt charIndex;

    Int64 glyph_width;
    Int64 advance;
    Int64 x_off;
    Int64 y_off;

    // Draw glyps to buffer
    UInt32 totalAdvanceX = 0;

    UChar8 *buffer = (UChar8*)calloc(1, bufferSize.area() * 4);

    while (*character != 0)
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            character++;
            continue;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT) != 0)
        {
            LLog::error("Failed to load FT_Face default.");
            character++;
            continue;
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

    free(utf32);

    LTexture *texture = new LTexture();

    if (!texture->setDataB(bufferSize, bufferSize.w()*4, DRM_FORMAT_ABGR8888, buffer))
    {
        if (clippedText != text)
            delete[] clippedText;

        free(buffer);
        delete texture;
        return nullptr;
    }

    free(buffer);

    if (clippedText != text)
        delete[] clippedText;

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
    UChar32 *utf32 = toUTF32(text);
    UChar32 *character = utf32;

    if (!character)
        return 0;

    while (*character != 0)
    {
        charIndex = FT_Get_Char_Index(face, (FT_ULong)*character);

        if (charIndex == 0)
        {
            LLog::error("Failed to get FT_Face char index.");
            character++;
            continue;
        }

        if (FT_Load_Glyph(face, charIndex, FT_LOAD_ADVANCE_ONLY) != 0)
        {
            LLog::error("Failed to load FT_Face advance.");
            character++;
            continue;
        }

        bufferSize.setW(bufferSize.w() + (face->glyph->metrics.horiAdvance >> 6));

        character++;
    }

    free(utf32);

    return bufferSize;
}

UChar32 *TextRenderer::toUTF32(const char *utf8Str)
{
    if (!utf8Str)
        return nullptr;

    if (*utf8Str == '\0')
        return nullptr;

    UErrorCode errorCode = U_ZERO_ERROR;

    Int32 utf8Len = strlen(utf8Str) + 1;
    Int32 str16Capacity = utf8Len;
    UChar *utf16Str = new UChar[str16Capacity * 2];
    Int32 utf16Len = 0;

    // Convert UTF-8 to UTF-16
    u_strFromUTF8(utf16Str,
                  str16Capacity,
                  &utf16Len,
                  utf8Str,
                  utf8Len,
                  &errorCode);

    if (U_FAILURE(errorCode))
    {
        delete[] utf16Str;
        LLog::error("Error converting UTF-8 to UTF-32: %s\n", u_errorName(errorCode));
        return nullptr;
    }

    Int32 str32Capacity = utf16Len + 1;

    UChar32 *utf32Str = (UChar32*)malloc(str32Capacity * 4);
    Int32 utf32Len = 0;

    u_strToUTF32(utf32Str,
                 str32Capacity,
                 &utf32Len,
                 utf16Str,
                 utf16Len,
                 &errorCode);

    if (U_FAILURE(errorCode))
    {
        delete[] utf16Str;
        LLog::error("Error converting UTF-8 to UTF-32: %s\n", u_errorName(errorCode));
        free(utf32Str);
        return nullptr;
    }

    utf32Str[utf32Len - 1] = 0;
    delete[] utf16Str;
    return utf32Str;
}
