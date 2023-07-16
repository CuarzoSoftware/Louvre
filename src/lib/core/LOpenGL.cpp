#include <LOpenGL.h>
#include <stdio.h>
#include <stdlib.h>
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

        LLog::error("%s",errorLog);

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
        LLog::error("Failed to load image %s. Unknown format.", file);
        return nullptr;
    }

    FIBITMAP *bitmap = FreeImage_Load(format, file);

    if (!bitmap)
    {
        LLog::error("Failed to load image %s.", file);
        return nullptr;
    }

    FreeImage_FlipVertical(bitmap);
    FIBITMAP *convertedBitmap = FreeImage_ConvertTo32Bits(bitmap);
    FreeImage_Unload(bitmap);

    if (!convertedBitmap)
    {
        LLog::error("Failed to convert image %s to 32 bit format.", file);
        return nullptr;
    }

    LSize size;
    size.setW(FreeImage_GetWidth(convertedBitmap));
    size.setH(FreeImage_GetHeight(convertedBitmap));
    UInt32 pitch = FreeImage_GetPitch(convertedBitmap);
    UChar8 *pixels = (UChar8*)FreeImage_GetBits(convertedBitmap);
    LTexture *texture = new LTexture();
    texture->setDataB(size, pitch, DRM_FORMAT_ARGB8888, pixels);

    FreeImage_Unload(convertedBitmap);

    return texture;
}
