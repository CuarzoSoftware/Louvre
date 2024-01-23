#include <private/LPainterPrivate.h>
#include <private/LOutputFramebufferPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>

#include <LRect.h>
#include <LOutput.h>
#include <LOpenGL.h>
#include <LLog.h>

#include <GLES2/gl2.h>
#include <cstdio>
#include <string.h>

using namespace Louvre;

static void makeExternalShader(std::string &shader)
{
    size_t pos = 0;
    std::string findStr = "sampler2D";
    std::string replaceStr = "samplerExternalOES";

    shader = std::string("#extension GL_OES_EGL_image_external : require\n") + shader;

    while ((pos = shader.find(findStr, pos)) != std::string::npos)
    {
        shader.replace(pos, findStr.length(), replaceStr);
        pos += replaceStr.length();
    }
}

void LPainter::bindTextureMode(const TextureParams &p)
{
    GLenum target = p.texture->target();
    imp()->switchTarget(target);

    Float32 fbScale;

    if (imp()->fb->type() == LFramebuffer::Output)
    {
        LOutputFramebuffer *outputFB = (LOutputFramebuffer*)imp()->fb;

        if (outputFB->output()->usingFractionalScale())
        {
            if (outputFB->output()->fractionalOversamplingEnabled())
            {
                fbScale = imp()->fb->scale();
            }
            else
            {
                fbScale = outputFB->output()->fractionalScale();
            }
        }
        else
        {
            fbScale = imp()->fb->scale();
        }
    }
    else
    {
        fbScale = imp()->fb->scale();
    }

    LPoint pos = p.pos - imp()->fb->rect().pos();
    Float32 srcDstX, srcDstY;
    Float32 srcW, srcH;
    Float32 srcDstW, srcDstH;
    Float32 srcFbX1, srcFbY1, srcFbX2, srcFbY2;
    Float32 srcFbW, srcFbH;
    Float32 srcRectW = p.srcRect.w() <= 0.f ? 0.001f : p.srcRect.w();
    Float32 srcRectH = p.srcRect.h() <= 0.f ? 0.001f : p.srcRect.h();

    bool xFlip = false;
    bool yFlip = false;

    LFramebuffer::Transform invTrans = LFramebuffer::requiredTransform(p.srcTransform, imp()->fb->transform());
    bool rotate = LFramebuffer::is90Transform(invTrans);

    if (LFramebuffer::is90Transform(p.srcTransform))
    {
        srcH = Float32(p.texture->sizeB().w()) / p.srcScale;
        srcW = Float32(p.texture->sizeB().h()) / p.srcScale;
        yFlip = !yFlip;
        xFlip = !xFlip;
    }
    else
    {
        srcW = (Float32(p.texture->sizeB().w()) / p.srcScale);
        srcH = (Float32(p.texture->sizeB().h()) / p.srcScale);
    }

    srcDstW = (Float32(p.dstSize.w()) * srcW) / srcRectW;
    srcDstX = (Float32(p.dstSize.w()) * p.srcRect.x()) / srcRectW;
    srcDstH = (Float32(p.dstSize.h()) * srcH) / srcRectH;
    srcDstY = (Float32(p.dstSize.h()) * p.srcRect.y()) / srcRectH;

    switch (invTrans)
    {
    case LFramebuffer::Normal:
        break;
    case LFramebuffer::Rotated90:
        xFlip = !xFlip;
        break;
    case LFramebuffer::Rotated180:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case LFramebuffer::Rotated270:
        yFlip = !yFlip;
        break;
    case LFramebuffer::Flipped:
        xFlip = !xFlip;
        break;
    case LFramebuffer::Flipped90:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case LFramebuffer::Flipped180:
        yFlip = !yFlip;
        break;
    case LFramebuffer::Flipped270:
        break;
    default:
        return;
    }

    Float32 screenH = Float32(imp()->fb->rect().h());
    Float32 screenW = Float32(imp()->fb->rect().w());

    switch (imp()->fb->transform())
    {
    case LFramebuffer::Normal:
        srcFbX1 = pos.x() - srcDstX;
        srcFbX2 = srcFbX1 + srcDstW;

        if (imp()->fbId == 0)
        {
            srcFbY1 = screenH - pos.y() + srcDstY;
            srcFbY2 = srcFbY1 - srcDstH;
        }
        else
        {
            srcFbY1 = pos.y() - srcDstY;
            srcFbY2 = srcFbY1 + srcDstH;
        }
        break;
    case LFramebuffer::Rotated90:
        srcFbX2 = pos.y() - srcDstY;
        srcFbX1 = srcFbX2 + srcDstH;

        if (imp()->fbId == 0)
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
        }
        else
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
            yFlip = !yFlip;
        }
        break;
    case LFramebuffer::Rotated180:
        srcFbX2 = screenW - pos.x() + srcDstX;
        srcFbX1 = srcFbX2 - srcDstW;

        if (imp()->fbId == 0)
        {
            srcFbY2 = pos.y() - srcDstY;
            srcFbY1 = srcFbY2 + srcDstH;
        }
        else
        {
            srcFbY2 = screenH - pos.y() + srcDstY;
            srcFbY1 = srcFbY2 - srcDstH;
        }
        break;
    case LFramebuffer::Rotated270:
        srcFbX1 = screenH - pos.y() + srcDstY;
        srcFbX2 = srcFbX1 - srcDstH;

        if (imp()->fbId == 0)
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
        }
        else
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
            yFlip = !yFlip;
        }
        break;
    case LFramebuffer::Flipped:
        srcFbX2 = screenW - pos.x() + srcDstX;
        srcFbX1 = srcFbX2 - srcDstW;

        if (imp()->fbId == 0)
        {
            srcFbY1 = screenH - pos.y() + srcDstY;
            srcFbY2 = srcFbY1 - srcDstH;
        }
        else
        {
            srcFbY1 = pos.y() - srcDstY;
            srcFbY2 = srcFbY1 + srcDstH;
        }
        break;
    case LFramebuffer::Flipped90:
        srcFbX2 = pos.y() - srcDstY;
        srcFbX1 = srcFbX2 + srcDstH;

        if (imp()->fbId == 0)
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
        }
        else
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
            yFlip = !yFlip;
        }
        break;
    case LFramebuffer::Flipped180:
        if (imp()->fbId == 0)
        {
            srcFbX1 = pos.x() - srcDstX;
            srcFbY1 = pos.y() - srcDstY + srcDstH;
            srcFbX2 = pos.x() - srcDstX + srcDstW;
            srcFbY2 = pos.y() - srcDstY;
        }
        else
        {
            srcFbY2 = screenH - pos.y() + srcDstY;
            srcFbY1 = srcFbY2 - srcDstH;
            srcFbX1 = pos.x() - srcDstX;
            srcFbX2 = pos.x() - srcDstX + srcDstW;
        }
        break;
    case LFramebuffer::Flipped270:
        srcFbX1 = screenH - pos.y() + srcDstY;
        srcFbX2 = srcFbX1 - srcDstH;

        if (imp()->fbId == 0)
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
        }
        else
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
            yFlip = !yFlip;
        }
        break;
    default:
        return;
    }

    imp()->shaderSetTransform(rotate);

    if (xFlip)
    {
        srcW = srcFbX2;
        srcFbX2 = srcFbX1;
        srcFbX1 = srcW;
    }

    if (yFlip)
    {
        srcH = srcFbY2;
        srcFbY2 = srcFbY1;
        srcFbY1 = srcH;
    }

    srcFbX1 *= fbScale;
    srcFbY1 *= fbScale;

    srcFbW = srcFbX2 * fbScale - srcFbX1;
    srcFbH = srcFbY2 * fbScale - srcFbY1;

    imp()->srcRect.x = srcFbX1;
    imp()->srcRect.y = srcFbY1;

    imp()->srcRect.w = srcFbW;
    imp()->srcRect.h = srcFbH;

    glActiveTexture(GL_TEXTURE0);
    imp()->shaderSetMode(3);
    imp()->shaderSetActiveTexture(0);
    glBindTexture(target, p.texture->id(imp()->output));
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void LPainter::bindColorMode()
{
    imp()->shaderSetMode(1);
}

void LPainter::drawBox(const LBox &box)
{
    imp()->setViewport(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawRect(const LRect &rect)
{
    imp()->setViewport(rect.x(), rect.y(), rect.w(), rect.h());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawRegion(const LRegion &region)
{
    Int32 n;
    LBox *box = region.boxes(&n);
    for (Int32 i = 0; i < n; i++)
    {
        imp()->setViewport(box->x1,
                           box->y1,
                           box->x2 - box->x1,
                           box->y2 - box->y1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        box++;
    }
}

void LPainter::enableCustomTextureColor(bool enabled)
{
    imp()->shaderSetTexColorEnabled(enabled);
}

bool LPainter::customTextureColorEnabled() const
{
    return imp()->currentState->texColorEnabled;
}

void LPainter::setAlpha(Float32 alpha)
{
    imp()->shaderSetAlpha(alpha);
}

void LPainter::setColor(const LRGBF &color)
{
    imp()->shaderSetColor(color.r, color.g, color.b);
}

LPainter::LPainter() : LPRIVATE_INIT_UNIQUE(LPainter)
{
    imp()->painter = this;

    compositor()->imp()->threadsMap[std::this_thread::get_id()].painter = this;

    imp()->updateExtensions();
    imp()->updateCPUFormats();

    // Open the vertex/fragment shaders
    GLchar vShaderStr[] = R"(
        precision mediump float;
        precision mediump int;
        uniform mediump vec2 texSize;
        uniform mediump vec4 srcRect;
        uniform mediump int transform;
        attribute mediump vec4 vertexPosition;
        varying mediump vec2 v_texcoord;
        uniform mediump int mode;

        void main()
        {
            if (mode == 3)
            {
                gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);

                if (vertexPosition.x == -1.0)
                    v_texcoord.x = srcRect.x;
                else
                    v_texcoord.x = srcRect.z;

                if (vertexPosition.y == 1.0)
                    v_texcoord.y = srcRect.y;
                else
                    v_texcoord.y = srcRect.w;

                if (transform == 1)
                    v_texcoord.yx = v_texcoord;

                return;
            }

            // Normal
            if (transform == 0)
                gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);

            // Counter270
            else if (transform == 3)
            {
                // TL > TR
                if (vertexPosition.x == -1.0 && vertexPosition.y == 1.0)
                    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
                // BL > TL
                else if (vertexPosition.x == -1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
                // BR > BL
                else if (vertexPosition.x == 1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
                // TR > BR
                else
                    gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
            }

            // Clock180
            else if (transform == 2)
            {
                // TL > BR
                if (vertexPosition.x == -1.0 && vertexPosition.y == 1.0)
                    gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
                // BL > TR
                else if (vertexPosition.x == -1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
                // BR > TL
                else if (vertexPosition.x == 1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
                // TR > BL
                else
                    gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
            }

            // Counter90
            else if (transform == 1)
            {
                // TL > BL
                if (vertexPosition.x == -1.0 && vertexPosition.y == 1.0)
                    gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
                // BL > BR
                else if (vertexPosition.x == -1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
                // BR > TR
                else if (vertexPosition.x == 1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
                // TR > TL
                else
                    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
            }

            // Flipped
            else if (transform == 4)
                gl_Position = vec4(-vertexPosition.x, vertexPosition.y, 0.0, 1.0);

            // Flipped270
            else if (transform == 7)
            {
                // TL > BR
                if (vertexPosition.x == -1.0 && vertexPosition.y == 1.0)
                    gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
                // BL > BL
                else if (vertexPosition.x == -1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
                // BR > TL
                else if (vertexPosition.x == 1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
                // TR > TR
                else
                    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
            }

            // Flipped180
            else if (transform == 6)
                gl_Position = vec4(vertexPosition.x, -vertexPosition.y, 0.0, 1.0);

            // Flipped90
            else if (transform == 5)
            {
                // TL > TL
                if (vertexPosition.x == -1.0 && vertexPosition.y == 1.0)
                    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
                // BL > TR
                else if (vertexPosition.x == -1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
                // BR > BR
                else if (vertexPosition.x == 1.0 && vertexPosition.y == -1.0)
                    gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
                // TR > BL
                else
                    gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
            }

            v_texcoord.x = (srcRect.x + vertexPosition.z*srcRect.z) / texSize.x;
            v_texcoord.y = (srcRect.y + srcRect.w - vertexPosition.w*srcRect.w) / texSize.y;
        }
        )";

    GLchar fShaderStr[] =R"(
        uniform mediump sampler2D tex;
        uniform bool colorFactorEnabled;
        uniform mediump int mode;
        uniform mediump float alpha;
        uniform mediump vec3 color;
        uniform mediump vec4 colorFactor;
        varying mediump vec2 v_texcoord;
        uniform bool texColorEnabled;

        void main()
        {
            // Texture
            if (mode == 3 || mode == 0)
            {
                if (texColorEnabled)
                {
                    gl_FragColor.xyz = color;
                    gl_FragColor.w = texture2D(tex, v_texcoord).w;
                }
                else
                    gl_FragColor = texture2D(tex, v_texcoord);

                gl_FragColor.w *= alpha;
            }

            // Solid color
            else if (mode == 1)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
            }

            if (colorFactorEnabled)
                gl_FragColor *= colorFactor;
        }
        )";

    std::string fShaderStrExternal = fShaderStr;
    makeExternalShader(fShaderStrExternal);

    GLchar fShaderStrScaler[] =R"(
        precision highp float;
        precision highp int;
        uniform highp sampler2D tex;
        uniform highp int mode;
        uniform highp vec4 samplerBounds;
        uniform highp vec2 pixelSize;
        uniform highp ivec2 iters;
        varying highp vec2 v_texcoord;

        void main()
        {
            vec2 texCoords;
            gl_FragColor = vec4(0.0);

            for (int x = 0; x < iters.x; x++)
            {
                texCoords.x = v_texcoord.x + float(x) * pixelSize.x;

                if (texCoords.x < samplerBounds.x)
                    texCoords.x = samplerBounds.x;
                else if (texCoords.x > samplerBounds.z)
                    texCoords.x = samplerBounds.z;

                for (int y = 0; y < iters.y; y++)
                {
                    texCoords.y = v_texcoord.y + float(y) * pixelSize.y;

                    if (texCoords.y < samplerBounds.y)
                        texCoords.y = samplerBounds.y;
                    else if (texCoords.y > samplerBounds.w)
                        texCoords.y = samplerBounds.w;

                    gl_FragColor += texture2D(tex, texCoords);
                }
            }

            gl_FragColor /= float(iters.x * iters.y);
        }
        )";

    std::string fShaderStrScalerExternal = fShaderStr;
    makeExternalShader(fShaderStrScalerExternal);

    imp()->vertexShader = LOpenGL::compileShader(GL_VERTEX_SHADER, vShaderStr);
    imp()->fragmentShader = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStr);
    imp()->fragmentShaderExternal = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrExternal.c_str());
    imp()->fragmentShaderScaler = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrScaler);
    imp()->fragmentShaderScalerExternal = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrScalerExternal.c_str());

    GLint linked;

    /************** SCALER PROGRAM **************/

    imp()->programObjectScaler = glCreateProgram();
    glAttachShader(imp()->programObjectScaler, imp()->vertexShader);
    glAttachShader(imp()->programObjectScaler, imp()->fragmentShaderScaler);

    // Link the program
    glLinkProgram(imp()->programObjectScaler);

    // Check the link status
    glGetProgramiv(imp()->programObjectScaler, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(imp()->programObjectScaler, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(imp()->programObjectScaler);
        imp()->programObjectScaler = 0;
        LLog::error("[LPainter::LPainter] Failed to compile scaler shader.");
    }
    else
    {
        imp()->currentProgram = imp()->programObjectScaler;
        imp()->currentUniformsScaler = &imp()->uniformsScaler;
        imp()->setupProgramScaler();
    }

    /************** SCALER PROGRAM EXTERNAL **************/

    imp()->programObjectScalerExternal = glCreateProgram();
    glAttachShader(imp()->programObjectScalerExternal, imp()->vertexShader);
    glAttachShader(imp()->programObjectScalerExternal, imp()->fragmentShaderScalerExternal);

    // Link the program
    glLinkProgram(imp()->programObjectScalerExternal);

    // Check the link status
    glGetProgramiv(imp()->programObjectScalerExternal, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(imp()->programObjectScalerExternal, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(imp()->programObjectScalerExternal);
        imp()->programObjectScalerExternal = 0;
        LLog::error("[LPainter::LPainter] Failed to compile scaler shader external.");
    }
    else
    {
        imp()->currentProgram = imp()->programObjectScalerExternal;
        imp()->currentUniformsScaler = &imp()->uniformsScalerExternal;
        imp()->setupProgramScaler();
    }

    /************** RENDER PROGRAM EXTERNAL **************/

    imp()->programObjectExternal = glCreateProgram();
    glAttachShader(imp()->programObjectExternal, imp()->vertexShader);
    glAttachShader(imp()->programObjectExternal, imp()->fragmentShaderExternal);

    // Link the program
    glLinkProgram(imp()->programObjectExternal);

    // Check the link status
    glGetProgramiv(imp()->programObjectExternal, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(imp()->programObjectExternal, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(imp()->programObjectExternal);
        LLog::error("[LPainter::LPainter] Failed to compile external OES shader.");
    }
    else
    {
        imp()->currentProgram = imp()->programObjectExternal;
#if LPAINTER_TRACK_UNIFORMS == 1
        imp()->currentState = &imp()->stateExternal;
#endif
        imp()->currentUniforms = &imp()->uniformsExternal;
        imp()->setupProgram();
    }

    /************** RENDER PROGRAM **************/

    imp()->programObject = glCreateProgram();
    glAttachShader(imp()->programObject, imp()->vertexShader);
    glAttachShader(imp()->programObject, imp()->fragmentShader);

    // Link the program
    glLinkProgram(imp()->programObject);

    // Check the link status
    glGetProgramiv(imp()->programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(imp()->programObject, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(imp()->programObject);
        exit(-1);
    }

    imp()->currentProgram = imp()->programObject;
#if LPAINTER_TRACK_UNIFORMS == 1
    imp()->currentState = &imp()->state;
#endif
    imp()->currentUniforms = &imp()->uniforms;
    imp()->setupProgram();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);

    imp()->shaderSetColorFactor(1.f, 1.f, 1.f, 1.f);
}

LPainter::~LPainter()
{
    glDeleteProgram(imp()->programObject);
    glDeleteProgram(imp()->programObjectExternal);
    glDeleteShader(imp()->fragmentShaderExternal);
    glDeleteShader(imp()->fragmentShader);
    glDeleteShader(imp()->vertexShader);
}

void LPainter::bindFramebuffer(LFramebuffer *framebuffer)
{
    if (!framebuffer)
    {
        imp()->fbId = 0;
        imp()->fb = nullptr;
        return;
    }

    imp()->fbId = framebuffer->id();
    glBindFramebuffer(GL_FRAMEBUFFER, imp()->fbId);
    imp()->fb = framebuffer;
}

LFramebuffer *LPainter::boundFramebuffer() const
{
    return imp()->fb;
}

void LPainter::LPainterPrivate::updateExtensions()
{
    openGLExtensions.EXT_read_format_bgra = LOpenGL::hasExtension("GL_EXT_read_format_bgra");
}

void LPainter::LPainterPrivate::updateCPUFormats()
{
    for (LDMAFormat *fmt : *compositor()->imp()->graphicBackend->getDMAFormats())
    {
        if (fmt->modifier != DRM_FORMAT_MOD_LINEAR)
            continue;

        switch (fmt->format)
        {
        case DRM_FORMAT_ARGB8888:
            cpuFormats.ARGB8888 = true;
            break;
        case DRM_FORMAT_XRGB8888:
            cpuFormats.XRGB8888 = true;
            break;
        case DRM_FORMAT_ABGR8888:
            cpuFormats.ABGR8888 = true;
            break;
        case DRM_FORMAT_XBGR8888:
            cpuFormats.XBGR8888 = true;
            break;
        default:
            break;
        }
    }
}

void LPainter::LPainterPrivate::setupProgram()
{
    glBindAttribLocation(currentProgram, 0, "vertexPosition");

    // Use the program object
    glUseProgram(currentProgram);

    // Load the vertex data
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, square);

    // Enables the vertex array
    glEnableVertexAttribArray(0);

    // Get Uniform Variables
    currentUniforms->texSize = glGetUniformLocation(currentProgram, "texSize");
    currentUniforms->srcRect = glGetUniformLocation(currentProgram, "srcRect");
    currentUniforms->activeTexture = glGetUniformLocation(currentProgram, "tex");
    currentUniforms->mode = glGetUniformLocation(currentProgram, "mode");
    currentUniforms->color= glGetUniformLocation(currentProgram, "color");
    currentUniforms->texColorEnabled = glGetUniformLocation(currentProgram, "texColorEnabled");
    currentUniforms->colorFactor = glGetUniformLocation(currentProgram, "colorFactor");
    currentUniforms->colorFactorEnabled = glGetUniformLocation(currentProgram, "colorFactorEnabled");
    currentUniforms->alpha = glGetUniformLocation(currentProgram, "alpha");
    currentUniforms->transform = glGetUniformLocation(currentProgram, "transform");
}

void LPainter::LPainterPrivate::setupProgramScaler()
{
    glBindAttribLocation(currentProgram, 0, "vertexPosition");

    // Use the program object
    glUseProgram(currentProgram);

    // Load the vertex data
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, square);

    // Enables the vertex array
    glEnableVertexAttribArray(0);

    // Get Uniform Variables
    currentUniformsScaler->texSize = glGetUniformLocation(currentProgram, "texSize");
    currentUniformsScaler->srcRect = glGetUniformLocation(currentProgram, "srcRect");
    currentUniformsScaler->activeTexture = glGetUniformLocation(currentProgram, "tex");
    currentUniformsScaler->pixelSize = glGetUniformLocation(currentProgram, "pixelSize");
    currentUniformsScaler->samplerBounds = glGetUniformLocation(currentProgram, "samplerBounds");
    currentUniformsScaler->iters = glGetUniformLocation(currentProgram, "iters");
}

void LPainter::drawTexture(const LTexture *texture,
                           const LRect &src, const LRect &dst,
                           Float32 srcScale, Float32 alpha)
{
    imp()->drawTexture(texture, src.x(), src.y(), src.w(), src.h(), dst.x(), dst.y(), dst.w(), dst.h(), srcScale, alpha);
}

void LPainter::drawTexture(const LTexture *texture,
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

    imp()->drawTexture(texture, srcX, srcY, srcW, srcH, dstX, dstY, dstW, dstH, srcScale, alpha);
}

void LPainter::drawColorTexture(const LTexture *texture, const LRGBF &color, const LRect &src, const LRect &dst, Float32 srcScale, Float32 alpha)
{
    imp()->drawColorTexture(texture,
                            color.r, color.g, color.b,
                            src.x(), src.y(), src.w(), src.h(),
                            dst.x(), dst.y(), dst.w(), dst.h(),
                            srcScale,
                            alpha);
}

void LPainter::drawColorTexture(const LTexture *texture,
                                Float32 r, Float32 g, Float32 b,
                                Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                                Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                                Float32 srcScale, Float32 alpha)
{

    imp()->drawColorTexture(texture, r, g, b,
                            srcX, srcY, srcW, srcH,
                            dstX, dstY, dstW, dstH,
                            srcScale, alpha);
}

void LPainter::drawColor(const LRect &dst,
                         Float32 r, Float32 g, Float32 b, Float32 a)
{
    imp()->drawColor(dst.x(), dst.y(), dst.w(), dst.h(),
                     r, g, b, a);
}

void LPainter::drawColor(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                         Float32 r, Float32 g, Float32 b, Float32 a)
{
    imp()->drawColor(dstX, dstY, dstW, dstH,
                     r, g, b, a);
}

void LPainter::setViewport(const LRect &rect)
{
    imp()->setViewport(rect.x(), rect.y(), rect.w(), rect.h());
}

void LPainter::setViewport(Int32 x, Int32 y, Int32 w, Int32 h)
{
    imp()->setViewport(x, y, w, h);
}

void LPainter::setClearColor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    glClearColor(r,g,b,a);
}

void LPainter::setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    imp()->shaderSetColorFactor(r, g, b, a);
}

void LPainter::clearScreen()
{
    if (!imp()->fb)
        return;

    glDisable(GL_BLEND);
    imp()->setViewport(imp()->fb->rect().x(), imp()->fb->rect().y(), imp()->fb->rect().w(), imp()->fb->rect().h());
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
}

void LPainter::bindProgram()
{
    glUseProgram(imp()->programObject);
}
