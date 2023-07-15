#ifndef LPAINTERPRIVATE_H
#define LPAINTERPRIVATE_H

#include <LPainter.h>
#include <GL/gl.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainter)
    GLuint vertexShader,fragmentShader;

    // Square (left for vertex, right for fragment)
    GLfloat square[16] =
    {  //  VERTEX       FRAGMENT
        -1.0f,  1.0f,   0.f, 1.f, // TL
        -1.0f, -1.0f,   0.f, 0.f, // BL
         1.0f, -1.0f,   1.f, 0.f, // BR
         1.0f,  1.0f,   1.f, 1.f  // TR
    };

    // Uniform variables
    GLuint
    texSizeUniform,             // Texture size (width,height)
    srcRectUniform,             // Src tex rect (x,y,width,height)
    activeTextureUniform,       // glActiveTexture
    modeUniform,
    colorUniform,
    alphaUniform,
    masksCountUniform,
    masksTypesUniform,
    masksRectsUniform,
    masksModesUniform,
    masksSamplesUniform,
    masksColorsUniform;

    struct LGLVec4F
    {
        GLfloat x, y, w, h;
    };

    LPainterMask *masks[LPAINTER_MAX_MASKS];
    GLint masksTypes[LPAINTER_MAX_MASKS];
    GLint masksModes[LPAINTER_MAX_MASKS];
    GLint masksSamples[LPAINTER_MAX_MASKS];
    LGLVec4F masksRects[LPAINTER_MAX_MASKS];
    LGLVec4F masksColors[LPAINTER_MAX_MASKS];
    GLint masksCount = 0;

    // Program
    GLuint programObject;
    LOutput *output                                     = nullptr;

    LPainter *painter;

    void scaleCursor(LTexture *texture, const LRect &src, const LRect &dst);
    void scaleTexture(LTexture *texture, const LRect &src, const LSize &dst);
    void bindMasks(Float32 dstX, Float32 dstY, Float32 dstW, Float32 dstH,
                   Float32 containerPosX, Float32 containerPosY);
};

#endif // LPAINTERPRIVATE_H
