#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <LNamespaces.h>
#include <GL/gl.h>

/*!
 * @brief Renderer compatible texture
 *
 * The LTexture class is an abstraction of an OpenGL texture.\n
 * It allows for generating textures from buffers in main memory or EGL images.
 */
class Louvre::LTexture
{
public:

    /*!
     * @brief Texture source
     */
    enum BufferSourceType
    {
        /// Buffer
        SHM = 0,

        /// EGL Image
        EGL = 1
    };

    /*!
     * @brief LTexture class constructor
     *
     * @param textureUnit Texture unit
     */
    LTexture(GLuint textureUnit = 1);

    /// LTexture class destructor
    ~LTexture();

    LTexture(const LTexture&) = delete;
    LTexture& operator= (const LTexture&) = delete;

    /*!
     * @brief Assigns the texture's content
     *
     * @param width Width of the source in buffer coordinates
     * @param height Height of the source in buffer coordinates
     * @param data Pixel source (buffer or EGL image)
     * @param format Pixel format
     * @param type Pixel data type
     * @param sourceType Source type (buffer or EGL image)
     */
    void setDataB(Int32 width, Int32 height, void *data, GLenum format = GL_BGRA, GLenum type = GL_UNSIGNED_BYTE, BufferSourceType sourceType = SHM);

    /*!
     * @brief Texture size in buffer coordinates
     */
    const LSize &sizeB() const;

    /*!
     * @brief Initialized property
     *
     * A texture is initialized when content has been assigned to it with setDataB().
     * @returns true if the texture has been initialized, false otherwise.
     */
    bool initialized();

    /*!
     * @brief OpenGL texture ID.
     */
    GLuint id();

    /*!
     * @brief OpenGL texture unit.
     */
    GLuint unit();

    /*!
     * @brief Pixels data type (e.g. GL_UNSIGNED_BYTE).
     */
    GLenum type();

    /*!
     * @brief Texture source.
     */
    BufferSourceType sourceType() const;

    /*!
     * @brief Pixels format (e.g. GL_RGBA)
     */
    GLenum format() const;

    class LTexturePrivate;

    /*!
     * @brief Access to the private API of LTexture.
     *
     * Returns an instance of the LTexturePrivate class (following the ***PImpl Idiom*** pattern) which contains all the private members of LSurfacePrivate.\n
     * Used internally by the library.
     */
    LTexturePrivate *imp() const;

private:
    LTexturePrivate *m_imp = nullptr;

};

#endif
