#ifndef LPAINTERPRIVATE_H
#define LPAINTERPRIVATE_H

#define LPAINTER_TRACK_UNIFORMS 1

#include <CZ/Louvre/Private/LTexturePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <LOutputFramebuffer.h>
#include <LPainter.h>
#include <LRect.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>

using namespace Louvre;

LPRIVATE_CLASS(LPainter)

enum ShaderMode : GLint
{
    LegacyMode = 0,
    TextureMode = 1,
    ColorMode = 2
};

struct Uniforms
{
    GLuint
        texSize,
        srcRect,
        activeTexture,
        mode,
        color,
        colorFactorEnabled,
        texColorEnabled,
        alpha,
        premultipliedAlpha,
        has90deg;
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

struct UserState
{
    TextureParams textureParams;
    ShaderMode mode { TextureMode };
    CZWeak<LTexture> texture;
    LBlendFunc customBlendFunc { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
    Float32 alpha { 1.f };
    LRGBF color { 1.f, 1.f, 1.f };
    LRGBAF colorFactor { 1.f, 1.f, 1.f, 1.f };
    bool autoBlendFunc { true };
    bool customTextureColor { false };
} userState;

LRectF srcRect;
bool needsBlendFuncUpdate { true };

static inline GLfloat square[]
{
    //  VERTEX     FRAGMENT
   -1.0f,  1.0f,   0.f, 1.f, // TL
   -1.0f, -1.0f,   0.f, 0.f, // BL
    1.0f, -1.0f,   1.f, 0.f, // BR
    1.0f,  1.0f,   1.f, 1.f  // TR
};

GLuint vertexShader, fragmentShader, fragmentShaderExternal, fragmentShaderScaler, fragmentShaderScalerExternal;

struct ShaderState
{
    LSizeF texSize;
    LRectF srcRect;
    GLuint activeTexture;
    ShaderMode mode;
    LRGBF color;
    bool colorFactorEnabled { false };
    bool texColorEnabled { false };
    bool premultipliedAlpha { false };
    bool has90deg { false };
    GLfloat alpha;
    GLfloat scale;
};

ShaderState state, stateExternal;
ShaderState *currentState;

// Program
GLuint programObject, programObjectExternal, programObjectScaler, programObjectScalerExternal, currentProgram;
LOutput *output = nullptr;
LPainter *painter;
LFramebuffer *fb = nullptr;
GLuint fbId = 0;
GLenum textureTarget = GL_TEXTURE_2D;

struct OpenGLExtensions
{
    bool EXT_read_format_bgra;
    bool OES_EGL_image;
} openGLExtensions;

void updateExtensions() noexcept;

struct CPUFormats
{
    bool ARGB8888 = false;
    bool XRGB8888 = false;
    bool ABGR8888 = false;
    bool XBGR8888 = false;
} cpuFormats;

void updateCPUFormats() noexcept;
void setupProgram() noexcept;
void setupProgramScaler() noexcept;

void shaderSetPremultipliedAlpha(bool premultipliedAlpha) noexcept
{
    if (currentState->premultipliedAlpha != premultipliedAlpha)
    {
        currentState->premultipliedAlpha = premultipliedAlpha;
        glUniform1i(currentUniforms->premultipliedAlpha, premultipliedAlpha);
    }
}

void shaderSetTexSize(const LSizeF &size) noexcept
{
    if (currentState->texSize != size)
    {
        currentState->texSize = size;
        glUniform2f(currentUniforms->texSize, size.w(), size.h());
    }
}

void shaderSetSrcRect(const LRectF &rect) noexcept
{
    if (currentState->srcRect != rect)
    {
        currentState->srcRect = rect;
        glUniform4f(currentUniforms->srcRect, rect.x(), rect.y(), rect.w(), rect.h());
    }
}

void shaderSetActiveTexture(GLuint unit) noexcept
{
    if (currentState->activeTexture != unit)
    {
        currentState->activeTexture = unit;
        glUniform1i(currentUniforms->activeTexture, unit);
    }
}

void shaderSetMode(ShaderMode mode) noexcept
{
    if (currentState->mode != mode)
    {
        currentState->mode = mode;
        glUniform1i(currentUniforms->mode, mode);
    }
}

void shaderSetColor(const LRGBF &color) noexcept
{
    if (currentState->color != color)
    {
        currentState->color = color;
        glUniform3f(currentUniforms->color, color.r, color.g, color.b);
    }
}

void shaderSetColorFactorEnabled(bool enabled) noexcept
{
    if (currentState->colorFactorEnabled != enabled)
    {
        currentState->colorFactorEnabled = enabled;
        glUniform1i(currentUniforms->colorFactorEnabled, enabled);
    }
}

void shaderSetTexColorEnabled(bool enabled) noexcept
{
    if (currentState->texColorEnabled != enabled)
    {
        currentState->texColorEnabled = enabled;
        glUniform1i(currentUniforms->texColorEnabled, enabled);
    }
}

void shaderSetHas90Deg(bool enabled) noexcept
{
    if (currentState->has90deg != enabled)
    {
        currentState->has90deg = enabled;
        glUniform1i(currentUniforms->has90deg, enabled);
    }
}

void shaderSetAlpha(Float32 a) noexcept
{
    if (currentState->alpha != a)
    {
        currentState->alpha = a;
        glUniform1f(currentUniforms->alpha, a);
    }
}

// GL params

void switchTarget(GLenum target) noexcept
{
    if (textureTarget != target)
    {
        if (target == GL_TEXTURE_2D)
        {
            currentProgram = programObject;
            currentUniforms = &uniforms;
            glUseProgram(currentProgram);
            currentState = &state;
            shaderSetColorFactorEnabled(stateExternal.colorFactorEnabled);
            shaderSetAlpha(stateExternal.alpha);
            shaderSetMode(stateExternal.mode);
            shaderSetColor(stateExternal.color);
            shaderSetTexColorEnabled(stateExternal.texColorEnabled);
            shaderSetPremultipliedAlpha(stateExternal.premultipliedAlpha);
            shaderSetHas90Deg(stateExternal.has90deg);
        }
        else
        {
            currentProgram = programObjectExternal;
            currentUniforms = &uniformsExternal;
            glUseProgram(currentProgram);
            currentState = &stateExternal;
            shaderSetColorFactorEnabled(state.colorFactorEnabled);
            shaderSetAlpha(state.alpha);
            shaderSetMode(state.mode);
            shaderSetColor(state.color);
            shaderSetTexColorEnabled(state.texColorEnabled);
            shaderSetPremultipliedAlpha(state.premultipliedAlpha);
            shaderSetHas90Deg(state.has90deg);
        }

        textureTarget = target;
    }
}

void setViewport(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
{
    x -= fb->rect().x();
    y -= fb->rect().y();

    if (fb->transform() == LTransform::Normal)
    {
    }
    else if (fb->transform() == LTransform::Rotated270)
    {
        Float32 tmp = x;
        x = fb->rect().h() - y - h;
        y = tmp;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (fb->transform() == LTransform::Rotated180)
    {
        x = fb->rect().w() - x - w;
        y = fb->rect().h() - y - h;
    }
    else if (fb->transform() == LTransform::Rotated90)
    {
        Float32 tmp = x;
        x = y;
        y = fb->rect().w() - tmp - w;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (fb->transform() == LTransform::Flipped)
    {
        x = fb->rect().w() - x - w;
    }
    else if (fb->transform() == LTransform::Flipped270)
    {
        Float32 tmp = x;
        x = fb->rect().h() - y - h;
        y = fb->rect().w() - tmp - w;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (fb->transform() == LTransform::Flipped180)
    {
        y = fb->rect().h() - y - h;
    }
    else if (fb->transform() == LTransform::Flipped90)
    {
        Float32 tmp = x;
        x = y;
        y = tmp;
        tmp = w;
        w = h;
        h = tmp;
    }

    if (fbId == 0)
    {
        if (Louvre::is90Transform(fb->transform()))
            y = fb->rect().w() - y - h;
        else
            y = fb->rect().h() - y - h;
    }

    Float32 fbScale;

    if (fb->type() == LFramebuffer::Output)
    {
        LOutputFramebuffer *outputFB = (LOutputFramebuffer*)fb;

        if (outputFB->output()->usingFractionalScale())
        {
            if (outputFB->output()->fractionalOversamplingEnabled())
                fbScale = fb->scale();
            else
                fbScale = outputFB->output()->fractionalScale();
        }
        else
            fbScale = fb->scale();
    }
    else
        fbScale = fb->scale();

    const Int32 x2 = floorf(Float32(x + w) * fbScale);
    const Int32 y2 = floorf(Float32(y + h) * fbScale);

    x = floorf(Float32(x) * fbScale);
    y = floorf(Float32(y) * fbScale);
    w = x2 - x;
    h = y2 - y;

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);

    if (currentState->mode == TextureMode)
    {
        shaderSetSrcRect(LRectF(
            (Float32(x) - srcRect.x()) / srcRect.w(),
            (Float32(y2) - srcRect.y()) / srcRect.h(),
            (Float32(x2) - srcRect.x()) / srcRect.w(),
            (Float32(y) - srcRect.y()) / srcRect.h()));
    }
}

void updateBlendingParams() noexcept
{
    needsBlendFuncUpdate = false;
    shaderSetMode(userState.mode);

    if (userState.mode == TextureMode)
    {
        /* Texture with replaced color */
        if (userState.customTextureColor)
        {
            shaderSetTexColorEnabled(true);
            LRGBF color { userState.color };
            const Float32 alpha { userState.alpha * userState.colorFactor.a};
            color.r *= userState.colorFactor.r;
            color.g *= userState.colorFactor.g;
            color.b *= userState.colorFactor.b;

            if (userState.autoBlendFunc)
            {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                    userState.customBlendFunc.dRGBFactor,
                                    userState.customBlendFunc.sAlphaFactor,
                                    userState.customBlendFunc.dAlphaFactor);
            }

            shaderSetColor(color);
            shaderSetAlpha(alpha);
        }

        /* Normal texture */
        else
        {
            shaderSetTexColorEnabled(false);

            /* Texture has premultiplied alpha */
            if (userState.texture.get() && userState.texture->premultipliedAlpha())
            {
                if (userState.autoBlendFunc)
                {
                    LRGBF colorFactor;
                    const Float32 alpha { userState.alpha * userState.colorFactor.a };
                    colorFactor.r = userState.colorFactor.r * alpha;
                    colorFactor.g = userState.colorFactor.g * alpha;
                    colorFactor.b = userState.colorFactor.b * alpha;
                    shaderSetPremultipliedAlpha(true);
                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    shaderSetColor(colorFactor);
                    shaderSetAlpha(alpha);
                }
                else
                {
                    const LRGBF colorFactor {
                        userState.colorFactor.r,
                        userState.colorFactor.g,
                        userState.colorFactor.b
                    };
                    const Float32 alpha { userState.alpha * userState.colorFactor.a };
                    shaderSetPremultipliedAlpha(false);
                    glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                        userState.customBlendFunc.dRGBFactor,
                                        userState.customBlendFunc.sAlphaFactor,
                                        userState.customBlendFunc.dAlphaFactor);
                    shaderSetColor(colorFactor);
                    shaderSetAlpha(alpha);
                }
            }
            else
            {
                const LRGBF colorFactor {
                    userState.colorFactor.r,
                    userState.colorFactor.g,
                    userState.colorFactor.b
                };
                const Float32 alpha { userState.alpha * userState.colorFactor.a };
                shaderSetPremultipliedAlpha(false);

                if (userState.autoBlendFunc)
                {
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                }
                else
                {
                    glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                        userState.customBlendFunc.dRGBFactor,
                                        userState.customBlendFunc.sAlphaFactor,
                                        userState.customBlendFunc.dAlphaFactor);
                }

                shaderSetColor(colorFactor);
                shaderSetAlpha(alpha);
            }
        }
    }

    /* Solid color mode */
    else
    {
        LRGBF color { userState.color };
        Float32 alpha { userState.alpha };

        alpha *= userState.colorFactor.a;
        color.r *= userState.colorFactor.r;
        color.g *= userState.colorFactor.g;
        color.b *= userState.colorFactor.b;

        if (userState.autoBlendFunc)
        {
            color.r *= alpha;
            color.g *= alpha;
            color.b *= alpha;
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                userState.customBlendFunc.dRGBFactor,
                                userState.customBlendFunc.sAlphaFactor,
                                userState.customBlendFunc.dAlphaFactor);
        }

        shaderSetColor(color);
        shaderSetAlpha(alpha);
    }
}
};

#endif // LPAINTERPRIVATE_H
