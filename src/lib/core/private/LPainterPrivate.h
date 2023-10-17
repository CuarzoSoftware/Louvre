#ifndef LPAINTERPRIVATE_H
#define LPAINTERPRIVATE_H

#include <LPainter.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainter)
    GLuint vertexShader, fragmentShader, fragmentShaderExternal;

    // Square (left for vertex, right for fragment)
    GLfloat square[16] =
    {  //  VERTEX       FRAGMENT
        -1.0f,  1.0f,   0.f, 1.f, // TL
        -1.0f, -1.0f,   0.f, 0.f, // BL
         1.0f, -1.0f,   1.f, 0.f, // BR
         1.0f,  1.0f,   1.f, 1.f  // TR
    };

    // Uniform variables
    struct Uniforms
    {
        GLuint
        texSize,
        srcRect,
        activeTexture,
        mode,
        color,
        colorFactor,
        alpha;
    } uniforms, uniformsExternal;

    Uniforms *currentUniforms;

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
        LGLVec4F colorFactor;
        GLfloat alpha;
    };

    ShaderState state, stateExternal;
    ShaderState *currentState;

    // Program
    GLuint programObject, programObjectExternal, currentProgram;
    LOutput *output = nullptr;

    LPainter *painter;

    void setupProgram();
    void scaleCursor(LTexture *texture, const LRect &src, const LRect &dst);
    void scaleTexture(LTexture *texture, const LRect &src, const LSize &dst);

    // Shader state update

    inline void shaderSetTexSize(Int32 w, Int32 h)
    {
        if (currentState->texSize.w != w || currentState->texSize.h != h)
        {
            currentState->texSize.w = w;
            currentState->texSize.h = h;
            glUniform2f(currentUniforms->texSize, w, h);
        }
    }

    inline void shaderSetSrcRect(Int32 x, Int32 y, Int32 w, Int32 h)
    {
        if (currentState->srcRect.x != x ||
            currentState->srcRect.y != y ||
            currentState->srcRect.w != w ||
            currentState->srcRect.h != h)
        {
            currentState->srcRect.x = x;
            currentState->srcRect.y = y;
            currentState->srcRect.w = w;
            currentState->srcRect.h = h;
            glUniform4f(currentUniforms->srcRect, x, y, w, h);
        }
    }

    inline void shaderSetActiveTexture(GLuint unit)
    {
        if (currentState->activeTexture != unit)
        {
            currentState->activeTexture = unit;
            glUniform1i(currentUniforms->activeTexture, unit);
        }
    }

    inline void shaderSetMode(GLint mode)
    {
        if (currentState->mode != mode)
        {
            currentState->mode = mode;
            glUniform1i(currentUniforms->mode, mode);
        }
    }

    inline void shaderSetColor(Float32 r, Float32 g, Float32 b)
    {
        if (currentState->color.r != r ||
            currentState->color.g != g ||
            currentState->color.b != b)
        {
            currentState->color.r = r;
            currentState->color.g = g;
            currentState->color.b = b;
            glUniform3f(currentUniforms->color, r, g, b);
        }
    }

    inline void shaderSetColorFactor(Float32 r, Float32 g, Float32 b, Float32 a)
    {
        if (currentState->colorFactor.x != r ||
            currentState->colorFactor.y != g ||
            currentState->colorFactor.w != b ||
            currentState->colorFactor.h != a)
        {
            currentState->colorFactor.x = r;
            currentState->colorFactor.y = g;
            currentState->colorFactor.w = b;
            currentState->colorFactor.h = a;
            glUniform4f(currentUniforms->colorFactor, r, g, b, a);
        }
    }

    inline void shaderSetAlpha(Float32 a)
    {
        if (currentState->alpha != a)
        {
            currentState->alpha = a;
            glUniform1f(currentUniforms->alpha, a);
        }
    }

    inline void switchTarget(GLenum target)
    {
        if (lastTarget != target)
        {
            if (target == GL_TEXTURE_2D)
            {
                currentProgram = programObject;
                currentState = &state;
                currentUniforms = &uniforms;
                glUseProgram(currentProgram);
                shaderSetColorFactor(stateExternal.colorFactor.x,
                                     stateExternal.colorFactor.y,
                                     stateExternal.colorFactor.w,
                                     stateExternal.colorFactor.h);
            }
            else
            {
                currentProgram = programObjectExternal;
                currentState = &stateExternal;
                currentUniforms = &uniformsExternal;
                glUseProgram(currentProgram);
                shaderSetColorFactor(state.colorFactor.x,
                                     state.colorFactor.y,
                                     state.colorFactor.w,
                                     state.colorFactor.h);
            }

            lastTarget = target;
        }
    }

    LFramebuffer *fb = nullptr;
    GLuint fbId = 0;
    GLenum lastTarget = GL_TEXTURE_2D;
};

#endif // LPAINTERPRIVATE_H
