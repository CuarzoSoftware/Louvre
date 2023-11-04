#ifndef LPAINTERPRIVATE_H
#define LPAINTERPRIVATE_H

#include <private/LTexturePrivate.h>
#include <LFramebuffer.h>
#include <LPainter.h>
#include <LRect.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainter)
    GLuint vertexShader, fragmentShader, fragmentShaderExternal, fragmentShaderScaler, fragmentShaderScalerExternal;

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
        colorFactorEnabled,
        alpha;
    } uniforms, uniformsExternal;

    Uniforms *currentUniforms;

    struct UniformsScaler
    {
        GLuint
            texSize,
            srcRect,
            activeTexture,
            pixelSize,
            samplerBounds,
            iters;
    } uniformsScaler, uniformsScalerExternal;

    UniformsScaler *currentUniformsScaler;

    struct LGLVec2F
    {
        GLfloat x, y;
    };

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
        GLint colorFactorEnabled;
        GLfloat alpha;
        LGLVec4F samplerBounds;
        LGLVec2F pixelSize;
        LGLSize iters;
    };

    ShaderState state, stateExternal;
    ShaderState *currentState;

    // Program
    GLuint programObject, programObjectExternal, programObjectScaler, programObjectScalerExternal, currentProgram;
    LOutput *output = nullptr;

    LPainter *painter;

    void setupProgram();
    void setupProgramScaler();

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
            shaderSetColorFactorEnabled(r != 1.f || g != 1.f || b != 1.f || a != 1.f);
        }
    }

    inline void shaderSetColorFactorEnabled(GLint enabled)
    {
        if (currentState->colorFactorEnabled != enabled)
        {
            currentState->colorFactorEnabled = enabled;
            glUniform1i(currentUniforms->colorFactorEnabled, enabled);
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

    /*
    inline void shaderSetPixelSize(Float32 x, Float32 y)
    {
        if (currentState->pixelSize.x != x ||
            currentState->pixelSize.y != y)
        {
            currentState->pixelSize.x = x;
            currentState->pixelSize.y = y;
            glUniform2f(currentUniforms->pixelSize, x, y);
        }
    }

    inline void shaderSetSamplerBounds(Float32 x1, Float32 y1, Float32 x2, Float32 y2)
    {
        if (currentState->samplerBounds.x != x1 ||
            currentState->samplerBounds.y != y1 ||
            currentState->samplerBounds.w != x2 ||
            currentState->samplerBounds.h != y2)
        {
            currentState->samplerBounds.x = x1;
            currentState->samplerBounds.y = y1;
            currentState->samplerBounds.w = x2;
            currentState->samplerBounds.h = y2;
            glUniform4f(currentUniforms->samplerBounds, x1, y1, x2, y2);
        }
    }

    inline void shaderSetIters(Int32 x, Int32 y)
    {
        if (currentState->iters.w != x ||
            currentState->iters.h != y)
        {
            currentState->iters.w = x;
            currentState->iters.h = y;
            glUniform2i(currentUniforms->iters, x, y);
        }
    }*/

    inline void setViewport(Int32 x, Int32 y, Int32 w, Int32 h)
    {
        x -= fb->rect().x();
        y -= fb->rect().y();

        if (fbId == 0)
            y = fb->rect().h() - y - h;

        Int32 fbScale = fb->scale();

        if (fbScale == 2)
        {
            x <<= 1;
            y <<= 1;
            w <<= 1;
            h <<= 1;
        }
        else if (fbScale > 2)
        {
            x *= fbScale;
            y *= fbScale;
            w *= fbScale;
            h *= fbScale;
        }

        glScissor(x, y, w, h);
        glViewport(x, y, w, h);
    }

    inline void drawTexture(const LTexture *texture,
                           Int32 srcX,
                           Int32 srcY,
                           Int32 srcW,
                           Int32 srcH,
                           Int32 dstX,
                           Int32 dstY,
                           Int32 dstW,
                           Int32 dstH,
                           Float32 srcScale,
                           Float32 alpha)
    {
        if (!texture || srcScale <= 0.f)
            return;

        if (alpha < 0.f)
            alpha = 0.f;

        GLenum target = texture->target();
        switchTarget(target);

        setViewport(dstX, dstY, dstW, dstH);
        glActiveTexture(GL_TEXTURE0);

        shaderSetAlpha(alpha);
        shaderSetMode(0);
        shaderSetActiveTexture(0);

        if (fbId != 0)
            shaderSetSrcRect(srcX, srcY + srcH, srcW, -srcH);
        else
            shaderSetSrcRect(srcX, srcY, srcW, srcH);

        glBindTexture(target, texture->id(output));
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (srcScale == 1.f)
            shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
        else if (srcScale == 2.f)
        {
            shaderSetTexSize(texture->sizeB().w() >> 1,
                             texture->sizeB().h() >> 1);
        }
        else
        {
            shaderSetTexSize(
                texture->sizeB().w()/srcScale,
                texture->sizeB().h()/srcScale);
        }

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    inline void drawColorTexture(const LTexture *texture,
                               Float32 r, Float32 g, Float32 b,
                               Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                               Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                               Float32 srcScale, Float32 alpha)
    {
        GLenum target = texture->target();
        switchTarget(target);

        setViewport(dstX, dstY, dstW, dstH);
        glActiveTexture(GL_TEXTURE0);

        shaderSetAlpha(alpha);
        shaderSetColor(r, g, b);
        shaderSetMode(2);
        shaderSetActiveTexture(0);

        if (fbId != 0)
            shaderSetSrcRect(srcX, srcY + srcH, srcW, -srcH);
        else
            shaderSetSrcRect(srcX, srcY, srcW, srcH);

        glBindTexture(target, texture->id(output));
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (srcScale == 1.f)
            shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
        else if (srcScale == 2.f)
        {
            shaderSetTexSize(texture->sizeB().w() >> 1,
                                    texture->sizeB().h() >> 1);
        }
        else
        {
            shaderSetTexSize(
                texture->sizeB().w()/srcScale,
                texture->sizeB().h()/srcScale);
        }

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    inline void drawColor(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                             Float32 r, Float32 g, Float32 b, Float32 a)
    {
        switchTarget(GL_TEXTURE_2D);
        setViewport(dstX, dstY, dstW, dstH);
        shaderSetAlpha(a);
        shaderSetColor(r, g, b);
        shaderSetMode(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    inline void scaleCursor(LTexture *texture, const LRect &src, const LRect &dst)
    {
        GLenum target = texture->target();
        GLuint textureId = texture->id(output);
        switchTarget(target);
        glDisable(GL_BLEND);
        glScissor(0,0,64,64);
        glViewport(0,0,64,64);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        glScissor(dst.x(),dst.y(),dst.w(),dst.h());
        glViewport(dst.x(),dst.y(),dst.w(),dst.h());
        glActiveTexture(GL_TEXTURE0);
        shaderSetAlpha(1.f);
        shaderSetMode(0);
        shaderSetActiveTexture(0);
        texture->imp()->setTextureParams(textureId, target, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
        shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
        shaderSetSrcRect(src.x(), src.y(), src.w(), src.h());
        shaderSetColorFactor(1.f, 1.f, 1.f, 1.f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    inline void scaleTexture(LTexture *texture, const LRect &src, const LSize &dst)
    {
        GLenum target = texture->target();
        GLuint textureId = texture->id(output);
        switchTarget(target);
        glDisable(GL_BLEND);
        glScissor(0, 0, dst.w(), dst.h());
        glViewport(0, 0, dst.w(), dst.h());
        glActiveTexture(GL_TEXTURE0);
        shaderSetAlpha(1.f);
        shaderSetMode(0);
        shaderSetActiveTexture(0);
        shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
        shaderSetSrcRect(src.x(), src.y() + src.h(), src.w(), -src.h());
        shaderSetColorFactor(1.f, 1.f, 1.f, 1.f);
        texture->imp()->setTextureParams(textureId, target, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    inline void scaleTexture(GLuint textureId, GLenum textureTarget, GLuint framebufferId, GLint minFilter, const LSize &texSize, const LRect &src, const LSize &dst)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
        switchTarget(textureTarget);
        glDisable(GL_BLEND);
        glScissor(0, 0, dst.w(), dst.h());
        glViewport(0, 0, dst.w(), dst.h());
        glActiveTexture(GL_TEXTURE0);
        shaderSetAlpha(1.f);
        shaderSetMode(0);
        shaderSetActiveTexture(0);
        shaderSetTexSize(texSize.w(), texSize.h());
        shaderSetSrcRect(src.x(), src.y() + src.h(), src.w(), -src.h());
        shaderSetColorFactor(1.f, 1.f, 1.f, 1.f);
        LTexture::LTexturePrivate::setTextureParams(textureId, textureTarget, GL_REPEAT, GL_REPEAT, minFilter, minFilter);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    LFramebuffer *fb = nullptr;
    GLuint fbId = 0;
    GLenum lastTarget = GL_TEXTURE_2D;
};

#endif // LPAINTERPRIVATE_H
