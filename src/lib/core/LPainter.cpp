#include <private/LPainterPrivate.h>
#include <private/LOutputFramebufferPrivate.h>
#include <private/LCompositorPrivate.h>

#include <GLES2/gl2.h>
#include <LOpenGL.h>
#include <LRect.h>
#include <LTexture.h>
#include <LOutput.h>
#include <cstdio>
#include <string.h>

using namespace Louvre;

LPainter::LPainter()
{
    m_imp = new LPainterPrivate();

    imp()->painter = this;

    compositor()->imp()->threadsMap[std::this_thread::get_id()].painter = this;

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
        uniform int mode;
        uniform lowp float alpha;
        uniform lowp vec3 color;
        varying lowp vec2 v_texcoord;

        void main()
        {
            if (mode == 0)
            {
                gl_FragColor = texture2D(tex, v_texcoord);
                gl_FragColor.w *= alpha;
                return;
            }

            if (mode == 1)
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
                return;
            }

            gl_FragColor.xyz = color;
            gl_FragColor.w = texture2D(tex, v_texcoord).w * alpha;
        }
        )";

    // Load the vertex/fragment shaders
    imp()->vertexShader = LOpenGL::compileShader(GL_VERTEX_SHADER, vShaderStr);
    imp()->fragmentShader = LOpenGL::compileShader(GL_FRAGMENT_SHADER, fShaderStr);

    // Create the program object
    imp()->programObject = glCreateProgram();
    glAttachShader(imp()->programObject, imp()->vertexShader);
    glAttachShader(imp()->programObject, imp()->fragmentShader);

    // Bind vPosition to attribute 0
    glBindAttribLocation(imp()->programObject, 0, "vertexPosition");

    // Link the program
    glLinkProgram(imp()->programObject);

    GLint linked;

    // Check the link status
    glGetProgramiv(imp()->programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(imp()->programObject, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(imp()->programObject);
        exit(-1);
    }

    // Enable alpha blending
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);

    // Set blend mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use the program object
    glUseProgram(imp()->programObject);

    // Load the vertex data
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, imp()->square);

    // Enables the vertex array
    glEnableVertexAttribArray(0);

    // Get Uniform Variables
    imp()->texSizeUniform = glGetUniformLocation(imp()->programObject, "texSize");
    imp()->srcRectUniform = glGetUniformLocation(imp()->programObject, "srcRect");
    imp()->activeTextureUniform = glGetUniformLocation(imp()->programObject, "tex");
    imp()->modeUniform = glGetUniformLocation(imp()->programObject, "mode");
    imp()->colorUniform = glGetUniformLocation(imp()->programObject, "color");
    imp()->alphaUniform = glGetUniformLocation(imp()->programObject, "alpha");
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

void LPainter::LPainterPrivate::scaleCursor(LTexture *texture, const LRect &src, const LRect &dst)
{
    glEnable(GL_BLEND);
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
    glBindTexture(GL_TEXTURE_2D, texture->id(output));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
    shaderSetSrcRect(src.x(), src.y(), src.w(), src.h());
    glBlendFunc(GL_ONE, GL_ZERO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::LPainterPrivate::scaleTexture(LTexture *texture, const LRect &src, const LSize &dst)
{
    glDisable(GL_BLEND);
    glScissor(0, 0, dst.w(), dst.h());
    glViewport(0, 0, dst.w(), dst.h());
    glActiveTexture(GL_TEXTURE0);
    shaderSetAlpha(1.f);
    shaderSetMode(0);
    shaderSetActiveTexture(0);
    shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
    shaderSetSrcRect(src.x(), src.y() + src.h(), src.w(), -src.h());
    glBindTexture(GL_TEXTURE_2D, texture->id(output));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawTexture(const LTexture *texture,
                            const LRect &src, const LRect &dst,
                            Float32 srcScale, Float32 alpha)
{
    drawTexture(texture,
                 src.x(),
                 src.y(),
                 src.w(),
                 src.h(),
                 dst.x(),
                 dst.y(),
                 dst.w(),
                 dst.h(),
                 srcScale,
                 alpha);
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
    if (!texture || srcScale <= 0.f)
        return;

    if (alpha < 0.f)
        alpha = 0.f;

    setViewport(dstX, dstY, dstW, dstH);
    glActiveTexture(GL_TEXTURE0);

    imp()->shaderSetAlpha(alpha);
    imp()->shaderSetMode(0);
    imp()->shaderSetActiveTexture(0);

    if (imp()->fbId != 0)
        imp()->shaderSetSrcRect(srcX, srcY + srcH, srcW, -srcH);
    else
        imp()->shaderSetSrcRect(srcX, srcY, srcW, srcH);

    glBindTexture(GL_TEXTURE_2D, texture->id(imp()->output));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (srcScale == 1.f)
        imp()->shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
    else if (srcScale == 2.f)
    {
        imp()->shaderSetTexSize(texture->sizeB().w() >> 1,
                                texture->sizeB().h() >> 1);
    }
    else
    {
        imp()->shaderSetTexSize(
                    texture->sizeB().w()/srcScale,
                    texture->sizeB().h()/srcScale);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawColorTexture(const LTexture *texture, const LRGBF &color, const LRect &src, const LRect &dst, Float32 srcScale, Float32 alpha)
{
    drawColorTexture(texture,
                     color.r, color.g, color.b,
                     src.x(),
                     src.y(),
                     src.w(),
                     src.h(),
                     dst.x(),
                     dst.y(),
                     dst.w(),
                     dst.h(),
                     srcScale,
                     alpha);
}

void LPainter::drawColorTexture(const LTexture *texture,
                                Float32 r, Float32 g, Float32 b,
                                Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                                Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                                Float32 srcScale, Float32 alpha)
{
    setViewport(dstX, dstY, dstW, dstH);
    glActiveTexture(GL_TEXTURE0);

    imp()->shaderSetAlpha(alpha);
    imp()->shaderSetColor(r, g, b);
    imp()->shaderSetMode(2);
    imp()->shaderSetActiveTexture(0);

    if (imp()->fbId != 0)
        imp()->shaderSetSrcRect(srcX, srcY + srcH, srcW, -srcH);
    else
        imp()->shaderSetSrcRect(srcX, srcY, srcW, srcH);

    glBindTexture(GL_TEXTURE_2D, texture->id(imp()->output));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (srcScale == 1.f)
        imp()->shaderSetTexSize(texture->sizeB().w(), texture->sizeB().h());
    else if (srcScale == 2.f)
    {
        imp()->shaderSetTexSize(texture->sizeB().w() >> 1,
                                texture->sizeB().h() >> 1);
    }
    else
    {
        imp()->shaderSetTexSize(
            texture->sizeB().w()/srcScale,
            texture->sizeB().h()/srcScale);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawColor(const LRect &dst,
                          Float32 r, Float32 g, Float32 b, Float32 a)
{
    drawColor(dst.x(), dst.y(), dst.w(), dst.h(),
               r, g, b, a);
}

void LPainter::drawColor(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                          Float32 r, Float32 g, Float32 b, Float32 a)
{
    setViewport(dstX, dstY, dstW, dstH);
    imp()->shaderSetAlpha(a);
    imp()->shaderSetColor(r, g, b);
    imp()->shaderSetMode(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::setViewport(const LRect &rect)
{
    setViewport(rect.x(), rect.y(), rect.w(), rect.h());
}

void LPainter::setViewport(Int32 x, Int32 y, Int32 w, Int32 h)
{
    x -= imp()->fb->rect().x();
    y -= imp()->fb->rect().y();

    if (imp()->fbId == 0)
        y = imp()->fb->rect().h() - y - h;

    if (imp()->fb->scale() == 2)
    {
        x <<= 1;
        y <<= 1;
        w <<= 1;
        h <<= 1;
    }
    else if (imp()->fb->scale() > 2)
    {
        x *= imp()->fb->scale();
        y *= imp()->fb->scale();
        w *= imp()->fb->scale();
        h *= imp()->fb->scale();
    }

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);
}

void LPainter::setClearColor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    glClearColor(r,g,b,a);
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
    glDeleteShader(imp()->fragmentShader);
    glDeleteShader(imp()->vertexShader);
    delete m_imp;
}
