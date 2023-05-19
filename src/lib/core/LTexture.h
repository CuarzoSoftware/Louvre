#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <LNamespaces.h>
#include <GL/gl.h>
#include <drm_fourcc.h>

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
        CPU = 0,
    };

    static UInt32 waylandFormatToDRM(UInt32 waylandFormat);

    LTexture(LCompositor *compositor, GLuint textureUnit = 1);

    bool setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer);
    bool updateRect(const LRect &rect, UInt32 stride, const void *buffer);

    /// LTexture class destructor
    ~LTexture();

    LTexture(const LTexture&) = delete;
    LTexture& operator= (const LTexture&) = delete;

    /*!
     * @brief Compositor instance
     */
    LCompositor *compositor() const;

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
    GLuint id(LOutput *output);

    /*!
     * @brief OpenGL texture unit.
     */
    GLuint unit();

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
