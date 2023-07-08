#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <LObject.h>
#include <GL/gl.h>
#include <LRect.h>
#include <drm_fourcc.h>

/*!
 * @brief Renderer compatible texture
 *
 * The LTexture class is an abstraction of an OpenGL texture.\n
 * It allows for generating textures from buffers in main memory or EGL images.
 */
class Louvre::LTexture : public LObject
{
public:

    /*!
     * @brief Texture source
     */
    enum BufferSourceType
    {
        /// Buffer
        CPU = 0,
        WL_DRM = 1,
        DMA = 2
    };

    static UInt32 waylandFormatToDRM(UInt32 waylandFormat);
    static UInt32 formatBytesPerPixel(UInt32 format);

    LTexture(GLuint textureUnit = 1);

    bool setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer);
    bool setData(void *wlDRMBuffer);
    bool setDataB(const LDMAPlanes *planes);
    bool updateRect(const LRect &rect, UInt32 stride, const void *buffer);

    LTexture *copyB(const LSize &dst = LSize(), const LRect &src = LRect()) const;

    /// LTexture class destructor
    ~LTexture();

    LTexture(const LTexture&) = delete;
    LTexture& operator= (const LTexture&) = delete;

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
    bool initialized() const;

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

    LPRIVATE_IMP(LTexture)
};

#endif
