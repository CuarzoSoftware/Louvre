#include "LOpenGL.h"
#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <LRect.h>
#include <LTexture.h>
#include <other/lodepng.h>
#include <LOutput.h>
#include <LLog.h>

using namespace Louvre;

char *LOpenGL::openShader(const char *file)
{
    FILE *fp;
    fp = fopen(file, "r");

    if (fp == NULL)
    {
        LLog::error("Error while opening shader file: %s.\n", file);
        return nullptr;
    }

    // Get file size
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    char *data = (char*)malloc(fileSize+1);

    size_t r = fread(data,fileSize,1,fp);
    (void)r;

    data[fileSize] = '\0';

    fclose(fp);
    return data;
}

bool LOpenGL::checkGLError(const char *msg)
{
    GLenum errCode;

    if ((errCode = glGetError()) != GL_NO_ERROR)
    {
        LLog::error("GL ERROR: %i %s\n", errCode, msg);
        return true;
    }

    return false;
}

GLuint LOpenGL::maxTextureUnits()
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&maxUnits);
    return maxUnits;
}

GLuint LOpenGL::compileShader(GLenum type, const char *shaderString)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    // Load the shader source
    glShaderSource(shader, 1, &shaderString, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        GLchar errorLog[infoLen];

        glGetShaderInfoLog(shader, infoLen, &infoLen, errorLog);

        LLog::error("%s",errorLog);

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

LTexture *LOpenGL::loadTexture(const char *pngFile, GLuint textureUnit)
{
    unsigned int error;
    unsigned char *image;
    unsigned int width, height;

    // Load PNG
    error = lodepng_decode32_file(&image, &width, &height, pngFile);

    if (error)
    {
        LLog::error("LOpenGL::loadTexture() failed to load PNG file: %s.", pngFile);
        return nullptr;
    }

    LSize s;
    s.setW(width);
    s.setH(height);
    LTexture *texture = new LTexture(textureUnit);
    texture->setDataB(s, width*4, DRM_FORMAT_ABGR8888, image);
    free(image);

    return texture;
}
