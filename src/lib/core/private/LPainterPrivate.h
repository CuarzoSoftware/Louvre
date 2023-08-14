#ifndef LPAINTERPRIVATE_H
#define LPAINTERPRIVATE_H

#include <LPainter.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainter)
    GLuint vertexShader, fragmentShader;

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

    struct LGLVec4F
    {
        GLfloat x, y, w, h;
    };

    struct LGLColor
    {
        GLfloat r, g, b;
    };

    struct LGLPoint
    {
        Int32 x, y;
    };

    struct LGLSize
    {
        Int32 w, h;
    };

    union LGLRect
    {
        Int32 x, y, w, h;
    };

    struct ShaderState
    {
        LGLSize texSize;
        LGLRect srcRect;
        GLuint activeTexture;
        GLint mode;
        LGLColor color;
        GLfloat alpha;
    };

    ShaderState state;

    // Program
    GLuint programObject;
    LOutput *output = nullptr;

    LPainter *painter;

    void scaleCursor(LTexture *texture, const LRect &src, const LRect &dst);
    void scaleTexture(LTexture *texture, const LRect &src, const LSize &dst);

    // Shader state update

    inline void shaderSetTexSize(Int32 w, Int32 h)
    {
        if (state.texSize.w != w || state.texSize.h != h)
        {
            state.texSize.w = w;
            state.texSize.h = h;
            glUniform2f(texSizeUniform, w, h);
        }
    }

    inline void shaderSetSrcRect(Int32 x, Int32 y, Int32 w, Int32 h)
    {
        if (state.srcRect.x != x ||
            state.srcRect.y != y ||
            state.srcRect.w != w ||
            state.srcRect.h != h)
        {
            state.srcRect.x = x;
            state.srcRect.y = y;
            state.srcRect.w = w;
            state.srcRect.h = h;
            glUniform4f(srcRectUniform, x, y, w, h);
        }
    }

    inline void shaderSetActiveTexture(GLuint unit)
    {
        if (state.activeTexture != unit)
        {
            state.activeTexture = unit;
            glUniform1i(activeTextureUniform, unit);
        }
    }

    inline void shaderSetMode(GLint mode)
    {
        if (state.mode != mode)
        {
            state.mode = mode;
            glUniform1i(modeUniform, mode);
        }
    }

    inline void shaderSetColor(Float32 r, Float32 g, Float32 b)
    {
        if (state.color.r != r ||
            state.color.g != g ||
            state.color.b != b)
        {
            state.color.r = r;
            state.color.g = g;
            state.color.b = b;
            glUniform3f(colorUniform, r, g, b);
        }
    }

    inline void shaderSetAlpha(Float32 a)
    {
        if (state.alpha != a)
        {
            state.alpha = a;
            glUniform1f(alphaUniform, a);
        }
    }

    LFramebuffer *fb = nullptr;
    GLuint fbId = 0;
};

#endif // LPAINTERPRIVATE_H
