#include "LLog.h"
#include <private/LPainterPrivate.h>

#include <GLES2/gl2.h>
#include <LOpenGL.h>
#include <LRect.h>
#include <LTexture.h>
#include <LOutput.h>
#include <cstdio>
#include <LCompositor.h>
#include <LPainterMask.h>
#include <string.h>

using namespace Louvre;

LPainter::LPainter()
{
    m_imp = new LPainterPrivate();

    imp()->painter = this;

    memset(&imp()->masks, 0, sizeof(imp()->masks));
    memset(&imp()->masksColors, 0, sizeof(imp()->masksColors));
    memset(&imp()->masksModes, 0, sizeof(imp()->masksModes));
    memset(&imp()->masksRects, 0, sizeof(imp()->masksRects));
    memset(&imp()->masksTypes, 0, sizeof(imp()->masksTypes));

    // Open the vertex/fragment shaders
    GLchar vShaderStr[] =
       "precision lowp float;\
        precision lowp int;\
        uniform vec2 texSize;\
        uniform vec4 srcRect;\
        uniform vec4 masksRects[4];\
        uniform int masksTypes[4];\
        uniform int masksCount;\
        attribute vec4 vertexPosition;\
        varying vec2 v_texcoord;\
        varying vec2 masks_texcoords[4];\
        void main()\
        {\
            gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);\
            v_texcoord.x = (srcRect.x + vertexPosition.z*srcRect.z) / texSize.x;\
            v_texcoord.y = (srcRect.y + srcRect.w - vertexPosition.w*srcRect.w) / texSize.y;\
            if (masksCount > 0)\
            {\
                for (int i = 0; i < 4; i++)\
                {\
                    if (masksTypes[i] != -1)\
                    {\
                        masks_texcoords[i] = masksRects[i].xy + masksRects[i].zw * vec2(vertexPosition.z, 1.0 - vertexPosition.w);\
                    }\
                }\
            }\
        }";

    GLchar fShaderStr[] =
       "precision lowp float;\
        precision lowp int;\
        uniform sampler2D tex;\
        uniform int mode;\
        uniform float alpha;\
        uniform vec4 colorRGBA;\
        uniform int masksCount;\
        uniform int masksTypes[4];\
        uniform int masksModes[4];\
        uniform vec4 masksColors[4];\
        varying vec2 v_texcoord;\
        varying vec2 masks_texcoords[4];\
        void main()\
        {\
          vec4 color;\
          if (mode == 0)\
          {\
            if (alpha == 1.0)\
                color = texture2D(tex, v_texcoord)*alpha;\
            else{\
                color = texture2D(tex, v_texcoord);\
                color.w *= alpha;\
            }\
          }\
          else\
            color = colorRGBA;\
\
          if (masksCount > 0)\
          {\
              int pendingMasks = masksCount;\
              for (int i = 0; i < 4; i++)\
              {\
                  if (masksTypes[i] == -1 || masks_texcoords[i].x < 0.0 || masks_texcoords[i].x > 1.0 || masks_texcoords[i].y < 0.0 || masks_texcoords[i].y > 1.0)\
                    continue;\
\
                  if(masksTypes[i] == 0)\
                    color.w = masksColors[i].w;\
                  else\
                    color = vec4(0.0);\
\
                  pendingMasks--;\
\
                  if (pendingMasks == 0)\
                    break;\
              }\
          }\
          gl_FragColor = color;\
        }";

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
    imp()->colorUniform = glGetUniformLocation(imp()->programObject, "colorRGBA");
    imp()->alphaUniform = glGetUniformLocation(imp()->programObject, "alpha");
    imp()->masksColorsUniform = glGetUniformLocation(imp()->programObject, "masksColors");
    imp()->masksCountUniform = glGetUniformLocation(imp()->programObject, "masksCount");
    imp()->masksTypesUniform = glGetUniformLocation(imp()->programObject, "masksTypes");
    imp()->masksRectsUniform = glGetUniformLocation(imp()->programObject, "masksRects");
    imp()->masksModesUniform = glGetUniformLocation(imp()->programObject, "masksModes");
    imp()->masksSamplesUniform = glGetUniformLocation(imp()->programObject, "masksSamples");
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
    glActiveTexture(GL_TEXTURE0 + texture->unit());
    glUniform1f(alphaUniform, 1.f);
    glUniform1i(modeUniform,0);
    glUniform1i(activeTextureUniform, texture->unit());
    glBindTexture(GL_TEXTURE_2D,texture->id(output));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform2f(texSizeUniform,texture->sizeB().w(), texture->sizeB().h());
    glUniform4f(srcRectUniform,src.x(), src.y(), src.w(), src.h());
    glUniform1i(masksCountUniform, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::LPainterPrivate::scaleTexture(LTexture *texture, const LRect &src, const LSize &dst)
{
    glEnable(GL_BLEND);
    glScissor(0, 0, dst.w(), dst.h());
    glViewport(0, 0, dst.w(), dst.h());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0 + texture->unit());
    glUniform1f(alphaUniform, 1.f);
    glUniform1i(modeUniform, 0);
    glUniform1i(activeTextureUniform, texture->unit());
    glBindTexture(GL_TEXTURE_2D, texture->id(output));
    glUniform2f(texSizeUniform, texture->sizeB().w(), texture->sizeB().h());
    glUniform4f(srcRectUniform, src.x(), src.y() + src.h(), src.w(), -src.h());
    glUniform1i(masksCountUniform, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::LPainterPrivate::bindMasks(Float32 dstX, Float32 dstY, Float32 dstW, Float32 dstH,
                                          Float32 containerPosX, Float32 containerPosY)
{
    masksCount = 0;
    UInt32 maxUnit = 0;
    for (UInt32 i = 0; i < LPAINTER_MAX_MASKS; i++)
    {
        if (masks[i])
        {
            masks[i]->bind(painter, i);
            Float32 x = (dstX - containerPosX - masksRects[i].x)/masksRects[i].w;
            Float32 y = (dstY - containerPosY - masksRects[i].y)/masksRects[i].h;
            masksRects[i].w = dstW/masksRects[i].w;
            masksRects[i].h = dstH/masksRects[i].h;
            masksRects[i].x = x;
            masksRects[i].y = y;
            masksCount++;

            if (i + 1 > maxUnit)
                maxUnit = i + 1;
        }
        else
            masksTypes[i] = -1;
    }

    glUniform1i(masksCountUniform, masksCount);
    glUniform1iv(masksTypesUniform, LPAINTER_MAX_MASKS, masksTypes);

    if (maxUnit)
    {
        glUniform4fv(masksRectsUniform, maxUnit, (GLfloat*)masksRects);
        glUniform1iv(masksModesUniform, maxUnit, masksModes);
        glUniform4fv(masksColorsUniform, maxUnit, (GLfloat*)masksColors);
        glUniform1iv(masksSamplesUniform, maxUnit, masksSamples);
    }
}

void LPainter::drawTextureC(LTexture *texture,
                            const LRect &src, const LRect &dst,
                            Float32 srcScale, Float32 alpha,
                            const LPoint &containerPos)
{
    drawTextureC(texture,
                 src.x(),
                 src.y(),
                 src.w(),
                 src.h(),
                 dst.x(),
                 dst.y(),
                 dst.w(),
                 dst.h(),
                 srcScale,
                 alpha,
                 containerPos.x(),
                 containerPos.y());
}

void LPainter::drawTextureC(LTexture *texture,
                            Int32 srcX,
                            Int32 srcY,
                            Int32 srcW,
                            Int32 srcH,
                            Int32 dstX,
                            Int32 dstY,
                            Int32 dstW,
                            Int32 dstH,
                            Float32 srcScale,
                            Float32 alpha,
                            Int32 containerPosX,
                            Int32 containerPosY)
{
    setViewportC(dstX, dstY, dstW, dstH);
    glActiveTexture(GL_TEXTURE0 + texture->unit());
    glUniform1f(imp()->alphaUniform, alpha);
    glUniform1i(imp()->modeUniform, 0);
    glUniform1i(imp()->activeTextureUniform, texture->unit());
    glBindTexture(GL_TEXTURE_2D, texture->id(imp()->output));
    glUniform4f(imp()->srcRectUniform, srcX, srcY, srcW, srcH);

    if (srcScale == 0.0)
    {
        glUniform2f(imp()->texSizeUniform, texture->sizeB().w(), texture->sizeB().h());
    }
    else
    {
        srcScale = float(compositor()->globalScale()) / srcScale;
        glUniform2f(imp()->texSizeUniform,
                    texture->sizeB().w()*srcScale,
                    texture->sizeB().h()*srcScale);
    }

    imp()->bindMasks(dstX, dstY, dstW, dstH,
                     containerPosX, containerPosY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::drawColorC(const LRect &dst,
                          Float32 r, Float32 g, Float32 b, Float32 a,
                          const LPoint &containerPos)
{
    drawColorC(dst.x(), dst.y(), dst.w(), dst.h(),
               r, g, b, a,
               containerPos.x(), containerPos.y());
}

void LPainter::drawColorC(Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                          Float32 r, Float32 g, Float32 b, Float32 a,
                          Int32 containerPosX, Int32 containerPosY)
{
    setViewportC(dstX, dstY, dstW, dstH);
    glUniform4f(imp()->colorUniform, r, g, b, a);
    glUniform1i(imp()->modeUniform, 1);
    imp()->bindMasks(dstX, dstY, dstW, dstH,
                     containerPosX, containerPosY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void LPainter::setViewportC(const LRect &rect)
{
    setViewportC(rect.x(), rect.y(), rect.w(), rect.h());
}

void LPainter::setViewportC(Int32 x, Int32 y, Int32 w, Int32 h)
{
    x -= imp()->output->posC().x();
    y -= imp()->output->posC().y();
    y = imp()->output->sizeC().h() - y - h;

    if (imp()->output->scale() != compositor()->globalScale())
    {
        x *= imp()->output->scale();
        y *= imp()->output->scale();
        w *= imp()->output->scale();
        h *= imp()->output->scale();

        x /= compositor()->globalScale();
        y /= compositor()->globalScale();
        w /= compositor()->globalScale();
        h /= compositor()->globalScale();
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
    glScissor(0,0,imp()->output->sizeB().w(),imp()->output->sizeB().h());
    glViewport(0,0,imp()->output->sizeB().w(),imp()->output->sizeB().h());
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
}

void LPainter::bindProgram()
{
    glUseProgram(imp()->programObject);
}

void LPainter::setMask(UInt32 slot, LPainterMask *mask)
{
    if (slot > LPAINTER_MAX_MASKS - 1)
        return;

    imp()->masks[slot] = mask;
}

LPainter::~LPainter()
{
    glDeleteProgram(imp()->programObject);
    glDeleteShader(imp()->fragmentShader);
    glDeleteShader(imp()->vertexShader);
    delete m_imp;
}
