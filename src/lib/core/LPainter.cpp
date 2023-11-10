#include "LLog.h"
#include <private/LPainterPrivate.h>
#include <private/LOutputFramebufferPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LTexturePrivate.h>

#include <GLES2/gl2.h>
#include <LOpenGL.h>
#include <LRect.h>
#include <LOutput.h>
#include <cstdio>
#include <string.h>

using namespace Louvre;

LPainter::LPainter()
{
    m_imp = new LPainterPrivate();
    imp()->painter = this;

    compositor()->imp()->threadsMap[std::this_thread::get_id()].painter = this;

    imp()->updateExtensions();
    imp()->updateCPUFormats();

    // Open the vertex/fragment shaders
    GLchar vShaderStr[] = R"(
        precision lowp float;
        precision lowp int;
        uniform lowp vec2 texSize;
        uniform lowp vec4 srcRect;
        attribute lowp vec4 vertexPosition;
        varying lowp vec2 v_texcoord;

        void main()
        {
            gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);
            v_texcoord.x = (srcRect.x + vertexPosition.z*srcRect.z) / texSize.x;
            v_texcoord.y = (srcRect.y + srcRect.w - vertexPosition.w*srcRect.w) / texSize.y;
        }
        )";

    GLchar fShaderStr[] =R"(
        precision lowp float;
        precision lowp int;
        uniform lowp sampler2D tex;

        uniform bool colorFactorEnabled;
        uniform lowp int mode;
        uniform lowp float alpha;
        uniform lowp vec3 color;
        uniform lowp vec4 colorFactor;
        varying lowp vec2 v_texcoord;

        void main()
        {
            // Texture
            if (mode == 0)
            {
                gl_FragColor = texture2D(tex, v_texcoord);
                gl_FragColor.w *= alpha;
            }

            // Solid color
            else if (mode == 1)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
            }

            // Colored texture
            else if (mode == 2)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = texture2D(tex, v_texcoord).w * alpha;
            }

            if (colorFactorEnabled)
                gl_FragColor *= colorFactor;
        }
        )";

    GLchar fShaderStrExternal[] =R"(
        #extension GL_OES_EGL_image_external : require
        precision lowp float;
        precision lowp int;
        uniform lowp samplerExternalOES tex;

        uniform bool colorFactorEnabled;
        uniform lowp int mode;
        uniform lowp float alpha;
        uniform lowp vec3 color;
        uniform lowp vec4 colorFactor;
        varying lowp vec2 v_texcoord;

        void main()
        {
            // Texture
            if (mode == 0)
            {
                gl_FragColor = texture2D(tex, v_texcoord);
                gl_FragColor.w *= alpha;
            }

            // Solid color
            else if (mode == 1)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
            }

            // Colored texture
            else if (mode == 2)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = texture2D(tex, v_texcoord).w * alpha;
            }

            if (colorFactorEnabled)
                gl_FragColor *= colorFactor;
        }
        )";

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

    GLchar fShaderStrScalerExternal[] =R"(
        #extension GL_OES_EGL_image_external : require
        precision highp float;
        precision highp int;
        uniform highp samplerExternalOES tex;
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

    // Load the vertex/fragment shaders
    imp()->vertexShader = LOpenGL::compileShader(GL_VERTEX_SHADER, vShaderStr);
    imp()->fragmentShader = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStr);
    imp()->fragmentShaderExternal = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrExternal);
    imp()->fragmentShaderScaler = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrScaler);
    imp()->fragmentShaderScalerExternal = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStrScalerExternal);

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
    currentUniforms->colorFactor = glGetUniformLocation(currentProgram, "colorFactor");
    currentUniforms->colorFactorEnabled = glGetUniformLocation(currentProgram, "colorFactorEnabled");
    currentUniforms->alpha = glGetUniformLocation(currentProgram, "alpha");
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
    glDisable(GL_BLEND);
    glScissor(0, 0, imp()->fb->sizeB().w(), imp()->fb->sizeB().h());
    glViewport(0, 0, imp()->fb->sizeB().w(), imp()->fb->sizeB().h());
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
}

void LPainter::bindProgram()
{
    glUseProgram(imp()->programObject);
}

LPainter::~LPainter()
{
    glDeleteProgram(imp()->programObject);
    glDeleteProgram(imp()->programObjectExternal);
    glDeleteShader(imp()->fragmentShaderExternal);
    glDeleteShader(imp()->fragmentShader);
    glDeleteShader(imp()->vertexShader);
    delete m_imp;
}
