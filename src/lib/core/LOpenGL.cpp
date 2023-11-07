#include <LOpenGL.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <LRect.h>
#include <LTexture.h>
#include <LOutput.h>
#include <LLog.h>
#include <FreeImage.h>

using namespace Louvre;

char *LOpenGL::openShader(const char *file)
{
    FILE *fp;
    fp = fopen(file, "r");

    if (fp == NULL)
    {
        LLog::error("[LOpenGL::openShader] Error while opening shader file: %s.\n", file);
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

const char *LOpenGL::glErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";

    case GL_TABLE_TOO_LARGE:
        return "GL_TABLE_TOO_LARGE";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
        return "GL_UNKNOWN_ERROR";
    }
}

GLuint LOpenGL::maxTextureUnits()
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits);
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

        LLog::error("[LOpenGL::compileShader] %s", errorLog);

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

LTexture *LOpenGL::loadTexture(const char *file)
{
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(file);

    if (format == FIF_UNKNOWN)
    {
        LLog::error("[LOpenGL::loadTexture] Failed to load image %s.", file);
        return nullptr;
    }

    FIBITMAP *bitmap = FreeImage_Load(format, file);

    if (!bitmap)
    {
        LLog::error("[LOpenGL::loadTexture] Failed to load image %s.", file);
        return nullptr;
    }

    FreeImage_FlipVertical(bitmap);
    FIBITMAP *convertedBitmap = FreeImage_ConvertTo32Bits(bitmap);
    FreeImage_Unload(bitmap);

    if (!convertedBitmap)
    {
        LLog::error("[LOpenGL::loadTexture] Failed to convert image %s to 32 bit format.", file);
        return nullptr;
    }

    LSize size((Int32)FreeImage_GetWidth(convertedBitmap), (Int32)FreeImage_GetHeight(convertedBitmap));
    UInt32 pitch = FreeImage_GetPitch(convertedBitmap);
    UChar8 *pixels = (UChar8*)FreeImage_GetBits(convertedBitmap);
    LTexture *texture = new LTexture();
    texture->setDataB(size, pitch, DRM_FORMAT_ARGB8888, pixels);
    FreeImage_Unload(convertedBitmap);
    return texture;
}
