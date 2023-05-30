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
    alphaUniform;

    // Program
    GLuint programObject;
    LOutput *output                                     = nullptr;

    void scaleCursor(LTexture *texture, const LRect &src, const LRect &dst);
};

#endif // LPAINTERPRIVATE_H
