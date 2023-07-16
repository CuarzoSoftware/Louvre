#ifndef LOPENGL_H
#define LOPENGL_H

#include <LNamespaces.h>

/*!
 * @brief OpenGL utility functions.
 *
 * Set of OpenGL ES 2.0 utility functions.
 */
class Louvre::LOpenGL
{
public:
    /*!
     * @brief Opens a GLSL shader file.
     *

     * @warning The returned string must be manually freed when no longer used.
     * @param fileName Path to the shader file.
     * @returns A string with the contents of the shader or nullptr in case of error.
     */
    static char *openShader(const char *fileName);

    /*!
     * @brief Checks for an OpenGL error.
     * @param msg Message to log if there is an error.
     * @returns true if an error exists (printing the error code and the message passed in the argument) or false if there is no error.
     */
    static bool checkGLError(const char *msg);

    /*!
     * @brief Maximum number of texture units.
     * @returns An integer with the maximum number of texture units supported by the machine.
     */
    static GLuint maxTextureUnits();

    /*!
     * @brief Builds a shader.
     *
     * @param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
     * @param shaderString String with the shader source code.
     * @returns The shader ID or 0 if an error occurs.
     */
    static GLuint compileShader(GLenum type, const char *shaderString);

    /*!
     * @brief Creates a texture from an image file.
     *
     * @param file Path to the image file. Must be an image format supported by FreeImage (JPEG, PNG, BMP, etc).
     * @returns A new texture or nullptr in case of error.
     */
    static LTexture *loadTexture(const char *file);
};

#endif // LOPENGL_H
