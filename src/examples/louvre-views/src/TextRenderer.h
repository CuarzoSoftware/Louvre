#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <freetype/freetype.h>
#include <LNamespaces.h>
#include <unicode/ustring.h>
#include <unicode/utf8.h>
#include <unicode/utf32.h>

using namespace Louvre;

class TextRenderer
{
public:
    static TextRenderer *loadFont(const char *fontName);
    ~TextRenderer();
    LTexture *renderText(const char *text, Int32 fontSize, Int32 maxWidth = -1, UChar8 r = 16, UChar8 g = 16, UChar8 b = 16);
    LSize calculateTextureSize(const char *text, Int32 fontSize);

    UChar32 *toUTF32(const char *utf8);

private:
    TextRenderer(const char *font);
    bool loadedFont = false;
    FT_Library ft;
    FT_Face face;
};

#endif // TEXTRENDERER_H
