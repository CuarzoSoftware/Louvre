#define STB_IMAGE_IMPLEMENTATION
#include <other/stb_image.h>
#include <LOpenGL.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <LRect.h>
#include <LTexture.h>
#include <LOutput.h>
#include <LLog.h>
#include <string.h>

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
    Int32 width, height, channels;
    UInt8 *image = stbi_load(file, &width, &height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        LLog::error("[LOpenGL::loadTexture] Failed to load image %s: %s.", file, stbi_failure_reason());
        return nullptr;
    }

    LTexture *texture = new LTexture();

    if (!texture->setDataB(LSize(width, height), width * 4, DRM_FORMAT_ABGR8888, image))
    {
        UInt8 pix = 0;

        for (int i = 0; i < width * 4 * height; i+=4)
        {
            pix = image[i + 2];
            image[i + 2] = image[i];
            image[i] = pix;
        }

        texture->setDataB(LSize(width, height), width * 4, DRM_FORMAT_ARGB8888, image);
    }

    free(image);
    return texture;
}

bool LOpenGL::hasExtension(const char *extension)
{
    const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
    size_t extlen = strlen(extension);
    const char *end = extensions + strlen(extensions);

    while (extensions < end)
    {
        if (*extensions == ' ')
        {
            extensions++;
            continue;
        }

        size_t n = strcspn(extensions, " ");

        if (n == extlen && strncmp(extension, extensions, n) == 0)
            return true;

        extensions += n;
    }

    return false;
}
