#include <cassert>
#include <private/LPainterPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>

#include <LRect.h>
#include <LOutput.h>
#include <LOpenGL.h>
#include <LLog.h>

#include <GLES2/gl2.h>

#if LOUVRE_USE_SKIA == 1
#include <include/gpu/gl/GrGLAssembleInterface.h>
#include <include/core/SkCanvas.h>
#else
#include <cstdio>
#include <string.h>
#endif

using namespace Louvre;

#if LOUVRE_USE_SKIA == 0
static void makeExternalShader(std::string &shader) noexcept
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
#endif

LPainter::LPainter() noexcept : LPRIVATE_INIT_UNIQUE(LPainter)
{
    imp()->painter = this;
    compositor()->imp()->threadsMap[std::this_thread::get_id()].painter = this;
    imp()->updateExtensions();
    imp()->updateCPUFormats();

#if LOUVRE_USE_SKIA == 1
    auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
        return (void *)eglGetProcAddress(p);
    });

    GrContextOptions skContextOptions;
    skContextOptions.fSuppressPrints = true;
    skContextOptions.fSkipGLErrorChecks = GrContextOptions::Enable::kYes;
    skContextOptions.fBufferMapThreshold = 0;
    skContextOptions.fDisableDistanceFieldPaths = true;
    skContextOptions.fAllowPathMaskCaching = true;
    skContextOptions.fDisableGpuYUVConversion = false;
    skContextOptions.fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;
    skContextOptions.fAvoidStencilBuffers = true;
    skContextOptions.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
    skContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
    skContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    skContextOptions.fRuntimeProgramCacheSize = 1024;
    skContextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendBinary;
    skContextOptions.fInternalMultisampleCount = 4;
    skContextOptions.fDisableTessellationPathRenderer = true;
    skContextOptions.fPreferExternalImagesOverES3 = true;
    skContextOptions.fReducedShaderVariations = false;
    skContextOptions.fAllowMSAAOnNewIntel = true;
    skContextOptions.fAlwaysUseTexStorageWhenAvailable = false;
    imp()->skContext = GrDirectContext::MakeGL(interface, skContextOptions);
    assert("[LPainter] Failed to create Skia context." && imp()->skContext != nullptr);
    imp()->skContext->setResourceCacheLimit(256000);
#else

    // Open the vertex/fragment shaders
    GLchar vShaderStr[] = R"(
        precision mediump float;
        precision mediump int;
        uniform mediump vec2 texSize;
        uniform mediump vec4 srcRect;
        attribute mediump vec4 vertexPosition;
        varying mediump vec2 v_texcoord;
        uniform lowp int mode;
        uniform bool has90deg;

        void main()
        {
            gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);

            if (mode == 1)
            {
                if (vertexPosition.x == -1.0)
                    v_texcoord.x = srcRect.x;
                else
                    v_texcoord.x = srcRect.z;

                if (vertexPosition.y == 1.0)
                    v_texcoord.y = srcRect.y;
                else
                    v_texcoord.y = srcRect.w;

                if (has90deg)
                    v_texcoord.yx = v_texcoord;

                return;
            }
            else if (mode == 0)
            {
                v_texcoord.x = (srcRect.x + vertexPosition.z*srcRect.z) / texSize.x;
                v_texcoord.y = (srcRect.y + srcRect.w - vertexPosition.w*srcRect.w) / texSize.y;
            }
        }
        )";

    GLchar fShaderStr[] =R"(
        uniform mediump sampler2D tex;
        uniform bool colorFactorEnabled;
        uniform lowp int mode;
        uniform mediump float alpha;
        uniform mediump vec3 color;
        varying mediump vec2 v_texcoord;
        uniform bool texColorEnabled;
        uniform bool premultipliedAlpha;

        void main()
        {
            // Texture
            if (mode != 2)
            {
                if (texColorEnabled)
                {
                    gl_FragColor.xyz = color;
                    gl_FragColor.w = texture2D(tex, v_texcoord).w;
                    if (alpha != 1.0)
                        gl_FragColor.w *= alpha;
                }
                else
                {
                    if (premultipliedAlpha)
                    {
                        gl_FragColor = texture2D(tex, v_texcoord);

                        if (alpha != 1.0)
                            gl_FragColor *= alpha;

                        if (colorFactorEnabled)
                            gl_FragColor.xyz *= color;
                    }
                    else
                    {
                        gl_FragColor = texture2D(tex, v_texcoord);

                        if (alpha != 1.0)
                            gl_FragColor.w *= alpha;

                        if (colorFactorEnabled)
                            gl_FragColor.xyz *= color;
                    }
                }
            }

            // Solid color
            else
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
            }
        }
        )";

    std::string fShaderStrExternal = fShaderStr;
    makeExternalShader(fShaderStrExternal);

    GLchar fShaderStrScaler[] =R"(
        precision highp float;
        precision highp int;
        uniform highp sampler2D tex;
        uniform lowp int mode;
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

    bindProgram();
#endif
}

LPainter::~LPainter() noexcept
{
    notifyDestruction();
#if LOUVRE_USE_SKIA == 1
    // TODO
#else
    glDeleteProgram(imp()->programObject);
    glDeleteProgram(imp()->programObjectExternal);
    glDeleteShader(imp()->fragmentShaderExternal);
    glDeleteShader(imp()->fragmentShader);
    glDeleteShader(imp()->vertexShader);
#endif
}

#if LOUVRE_USE_SKIA == 0
void LPainter::LPainterPrivate::setupProgram() noexcept
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
    currentUniforms->colorFactorEnabled = glGetUniformLocation(currentProgram, "colorFactorEnabled");
    currentUniforms->alpha = glGetUniformLocation(currentProgram, "alpha");
    currentUniforms->premultipliedAlpha = glGetUniformLocation(currentProgram, "premultipliedAlpha");
    currentUniforms->has90deg = glGetUniformLocation(currentProgram, "has90deg");
}

void LPainter::LPainterPrivate::setupProgramScaler() noexcept
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
#endif

void LPainter::LPainterPrivate::updateExtensions() noexcept
{
    const char *exts = (const char*)glGetString(GL_EXTENSIONS);
    openGLExtensions.EXT_read_format_bgra = LOpenGL::hasExtension(exts, "GL_EXT_read_format_bgra");
    openGLExtensions.OES_EGL_image = LOpenGL::hasExtension(exts, "GL_OES_EGL_image");
}

void LPainter::LPainterPrivate::updateCPUFormats() noexcept
{
    for (const LDMAFormat &fmt : *compositor()->imp()->graphicBackend->backendGetDMAFormats())
    {
        if (fmt.modifier != DRM_FORMAT_MOD_LINEAR)
            continue;

        switch (fmt.format)
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

void LPainter::bindTextureMode(const TextureParams &p) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.mode = LPainterPrivate::TextureMode;
    imp()->skUpdateTextureParams(p);
#else
    GLenum target = p.texture->target();
    imp()->switchTarget(target);

    if (imp()->userState.mode != LPainterPrivate::TextureMode)
    {
        imp()->userState.mode = LPainterPrivate::TextureMode;
        imp()->needsBlendFuncUpdate = true;
    }

    if (!imp()->userState.texture.get() || imp()->userState.texture.get()->premultipliedAlpha() != p.texture->premultipliedAlpha())
        imp()->needsBlendFuncUpdate = true;

    imp()->userState.texture = p.texture;

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

    LTransform invTrans = Louvre::requiredTransform(p.srcTransform, imp()->fb->transform());
    bool rotate = Louvre::is90Transform(invTrans);

    if (Louvre::is90Transform(p.srcTransform))
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
    case LTransform::Normal:
        break;
    case LTransform::Rotated90:
        xFlip = !xFlip;
        break;
    case LTransform::Rotated180:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case LTransform::Rotated270:
        yFlip = !yFlip;
        break;
    case LTransform::Flipped:
        xFlip = !xFlip;
        break;
    case LTransform::Flipped90:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case LTransform::Flipped180:
        yFlip = !yFlip;
        break;
    case LTransform::Flipped270:
        break;
    default:
        return;
    }

    Float32 screenH = Float32(imp()->fb->rect().h());
    Float32 screenW = Float32(imp()->fb->rect().w());

    switch (imp()->fb->transform())
    {
    case LTransform::Normal:
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
    case LTransform::Rotated90:
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
    case LTransform::Rotated180:
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
    case LTransform::Rotated270:
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
    case LTransform::Flipped:
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
    case LTransform::Flipped90:
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
    case LTransform::Flipped180:
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
    case LTransform::Flipped270:
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

    imp()->shaderSetHas90Deg(rotate);

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

    imp()->srcRect.setX(srcFbX1);
    imp()->srcRect.setY(srcFbY1);
    imp()->srcRect.setW(srcFbW);
    imp()->srcRect.setH(srcFbH);

    glActiveTexture(GL_TEXTURE0);
    imp()->shaderSetMode(LPainterPrivate::TextureMode);
    imp()->shaderSetActiveTexture(0);
    glBindTexture(target, p.texture->id(imp()->output));
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
}

void LPainter::bindColorMode() noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.mode = LPainterPrivate::ColorMode;
    imp()->skUpdateFbMatrix();
#else
    if (imp()->userState.mode == LPainterPrivate::ColorMode)
        return;

    imp()->userState.mode = LPainterPrivate::ColorMode;
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::drawBox(const LBox &box) noexcept
{
#if LOUVRE_USE_SKIA == 1
    SkPath path;
    path.addRect(box.x1, box.y1, box.x2, box.y2);
    imp()->skPaintPath(path);
#else
    if (imp()->needsBlendFuncUpdate)
        imp()->updateBlendingParams();

    imp()->setViewport(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
}

void LPainter::drawRect(const LRect &rect) noexcept
{
#if LOUVRE_USE_SKIA == 1
    SkPath path;
    path.addRect(SkRect::MakeXYWH(rect.x(), rect.y(), rect.w(), rect.h()));
    imp()->skPaintPath(path);
#else
    if (imp()->needsBlendFuncUpdate)
        imp()->updateBlendingParams();

    imp()->setViewport(rect.x(), rect.y(), rect.w(), rect.h());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
}

void LPainter::drawRegion(const LRegion &region) noexcept
{
#if LOUVRE_USE_SKIA == 1

    if (region.empty())
        return;

    Int32 n;
    const LBox *box { region.boxes(&n) };
    SkPath path;

    for (Int32 i = 0; i < n; i++)
        path.addRect(box[i].x1, box[i].y1, box[i].x2, box[i].y2);
    imp()->skPaintPath(path);
#else
    if (imp()->needsBlendFuncUpdate)
        imp()->updateBlendingParams();

    Int32 n;
    const LBox *box = region.boxes(&n);
    for (Int32 i = 0; i < n; i++)
    {
        imp()->setViewport(box->x1,
                           box->y1,
                           box->x2 - box->x1,
                           box->y2 - box->y1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        box++;
    }
#endif
}

void LPainter::enableCustomTextureColor(bool enabled) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.customTextureColor = enabled;
#else
    if (imp()->userState.customTextureColor == enabled)
        return;

    imp()->userState.customTextureColor = enabled;
    imp()->needsBlendFuncUpdate = true;
#endif
}

bool LPainter::customTextureColorEnabled() const noexcept
{
    return imp()->userState.customTextureColor;
}

bool LPainter::autoBlendFuncEnabled() const noexcept
{
    return imp()->userState.autoBlendFunc;
}

void LPainter::enableAutoBlendFunc(bool enabled) const noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.autoBlendFunc = enabled;
#else
    if (imp()->userState.autoBlendFunc == enabled)
        return;

    imp()->userState.autoBlendFunc = enabled;
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::setAlpha(Float32 alpha) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.alpha = alpha;
#else
    if (imp()->userState.alpha == alpha)
        return;

    imp()->userState.alpha = alpha;
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::setColor(const LRGBF &color) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.color = color;
#else
    if (imp()->userState.color == color)
        return;

    imp()->userState.color = color;
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::bindFramebuffer(LFramebuffer *framebuffer) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->fb.reset(framebuffer);
#else
    if (!framebuffer)
    {
        imp()->fbId = 0;
        imp()->fb = nullptr;
        return;
    }

    imp()->fbId = framebuffer->id();
    glBindFramebuffer(GL_FRAMEBUFFER, imp()->fbId);
    imp()->fb = framebuffer;
#endif
}

LFramebuffer *LPainter::boundFramebuffer() const noexcept
{
    return imp()->fb;
}

void LPainter::setViewport(const LRect &rect) noexcept
{
#if LOUVRE_USE_SKIA == 1
    L_UNUSED(rect)
#else
    imp()->setViewport(rect.x(), rect.y(), rect.w(), rect.h());
#endif
}

void LPainter::setViewport(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
{
#if LOUVRE_USE_SKIA == 1
    L_UNUSED(x)
    L_UNUSED(y)
    L_UNUSED(w)
    L_UNUSED(h)
#else
    imp()->setViewport(x, y, w, h);
#endif
}

void LPainter::setClearColor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->skClearColor.fR = r;
    imp()->skClearColor.fG = g;
    imp()->skClearColor.fB = b;
    imp()->skClearColor.fA = a;
#else
    glClearColor(r,g,b,a);
#endif
}

void LPainter::setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.colorFactor = {r, g, b, a};
#else
    if (imp()->userState.colorFactor.r == r &&
        imp()->userState.colorFactor.g == g &&
        imp()->userState.colorFactor.b == b &&
        imp()->userState.colorFactor.a == a)
        return;

    imp()->userState.colorFactor = {r, g, b, a};
    imp()->shaderSetColorFactorEnabled(r != 1.f || g != 1.f || b != 1.f || a != 1.f);
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::setColorFactor(const LRGBAF &factor) noexcept
{
#if LOUVRE_USE_SKIA == 1
    imp()->userState.colorFactor = factor;
#else
    if (imp()->userState.colorFactor == factor)
        return;

    imp()->userState.colorFactor = factor;
    imp()->shaderSetColorFactorEnabled(factor.r != 1.f || factor.g != 1.f || factor.b != 1.f || factor.a != 1.f);
    imp()->needsBlendFuncUpdate = true;
#endif
}

void LPainter::clearScreen() noexcept
{
#if LOUVRE_USE_SKIA == 1
    if (!imp()->fb)
        return;

    if (auto surf { imp()->fb->skSurface() })
        surf->getCanvas()->clear(imp()->skClearColor);
#else
    if (!imp()->fb)
        return;

    glDisable(GL_BLEND);
    imp()->setViewport(imp()->fb->rect().x(), imp()->fb->rect().y(), imp()->fb->rect().w(), imp()->fb->rect().h());
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
#endif
}

void LPainter::bindProgram() noexcept
{
#if LOUVRE_USE_SKIA == 1
    //imp()->skContext->resetContext();
#else
    eglBindAPI(EGL_OPENGL_ES_API);
    glUseProgram(imp()->currentProgram);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindAttribLocation(imp()->currentProgram, 0, "vertexPosition");
    glUseProgram(imp()->currentProgram);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, imp()->square);
    glEnableVertexAttribArray(0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_CLAMP);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glDisable(GL_RASTERIZER_DISCARD);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_SHADING);
    glDisable(GL_SAMPLE_MASK);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_COLOR_LOGIC_OP);
    glDisable(GL_INDEX_LOGIC_OP);
    glDisable(GL_COLOR_TABLE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisable(GL_MULTISAMPLE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glCullFace(GL_BACK);
    glLineWidth(1);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
    glPolygonOffset(0, 0);
    glDepthFunc(GL_LESS);
    glDepthRangef(0, 1);
    glStencilMask(1);
    glDepthMask(GL_FALSE);
    glFrontFace(GL_CCW);
    glBlendColor(0, 0, 0, 0);
    glBlendEquation(GL_FUNC_ADD);

    glUniform2f(imp()->currentUniforms->texSize,
        imp()->currentState->texSize.w(),
        imp()->currentState->texSize.h());

    glUniform4f(imp()->currentUniforms->srcRect,
        imp()->currentState->srcRect.x(),
        imp()->currentState->srcRect.y(),
        imp()->currentState->srcRect.w(),
        imp()->currentState->srcRect.h());

    glUniform1i(imp()->currentUniforms->activeTexture,
        imp()->currentState->activeTexture);

    glUniform1i(imp()->currentUniforms->colorFactorEnabled,
        imp()->currentState->colorFactorEnabled);

    glUniform1f(imp()->currentUniforms->alpha,
        imp()->currentState->alpha);

    glUniform1i(imp()->currentUniforms->mode,
        imp()->currentState->mode);

    glUniform3f(imp()->currentUniforms->color,
        imp()->currentState->color.r,
        imp()->currentState->color.g,
        imp()->currentState->color.b);

    glUniform1i(imp()->currentUniforms->texColorEnabled,
        imp()->currentState->texColorEnabled);

    glUniform1i(imp()->currentUniforms->premultipliedAlpha,
        imp()->currentState->premultipliedAlpha);

    glUniform1i(imp()->currentUniforms->has90deg,
        imp()->currentState->has90deg);

    imp()->updateBlendingParams();
#endif
}

#if LOUVRE_USE_SKIA == 1
GrDirectContext *LPainter::skContext() const noexcept
{
    return imp()->skContext.get();
}
#endif

void LPainter::setBlendFunc(const LBlendFunc &blendFunc) const noexcept
{
    imp()->userState.customBlendFunc = blendFunc;

    if (!imp()->userState.autoBlendFunc)
        glBlendFuncSeparate(blendFunc.sRGBFactor, blendFunc.dRGBFactor, blendFunc.sAlphaFactor, blendFunc.dAlphaFactor);
}
