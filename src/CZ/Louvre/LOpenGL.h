#ifndef LOPENGL_H
#define LOPENGL_H

#include <LNamespaces.h>
#include <filesystem>

/**
 * @brief OpenGL utility functions.
 *
 * Set of OpenGL ES 2.0 utility functions.
 */
class Louvre::LOpenGL
{
public:
    LOpenGL() = delete;

    LCLASS_NO_COPY(LOpenGL)

    /**
     * @brief Open a GLSL shader file.
     *
     * @note The returned string must be manually freed when no longer used.
     *
     * @param file Path to the shader file.
     * @returns A string with the contents of the shader or `nullptr` in case of error.
     */
    static char *openShader(const std::filesystem::path &file);

    /**
     * @brief Gets a string representation of an OpenGL error code.
     *
     * This function converts an OpenGL error code obtained from glGetError() into a human-readable string.
     *
     * @param error The OpenGL error code returned by glGetError().
     * @return A const char pointer containing the error message.
     */
    static const char* glErrorString(GLenum error);

    /**
     * @brief Maximum number of texture units.
     * @returns An integer with the maximum number of texture units supported by OpenGL.
     */
    static GLuint maxTextureUnits();

    /**
     * @brief Compile a shader.
     *
     * @param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
     * @param shaderString String with the shader source code.
     * @returns The shader ID or 0 if an error occurs.
     */
    static GLuint compileShader(GLenum type, const char *shaderString);

    /**
     * @brief Create a texture from an image file.
     *
     * Loads and creates a texture from an image file.
     *
     * @note The image format is always converted to `DRM_FORMAT_ARGB8888` or `DRM_FORMAT_ABGR8888`.
     *
     * @param file Path to the image file. Must be an image format supported by [STB Image](https://github.com/nothings/stb) (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC).
     * @returns A new texture or `nullptr` in case of error.
     */
    static LTexture *loadTexture(const std::filesystem::path &file);

    /**
     * @brief Check if a specific OpenGL extension is available.
     *
     * This function queries the list of supported OpenGL extensions and determines
     * whether the specified extension is among them.
     *
     * @param extension The name of the OpenGL extension to check.
     * @return `true` if the extension is available, `false` otherwise.
     */
    static bool hasExtension(const char *extensions, const char *extension);
};

#endif // LOPENGL_H
