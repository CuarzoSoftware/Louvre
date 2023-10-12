#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <LObject.h>
#include <LRect.h>
#include <drm_fourcc.h>

/**
 * @brief OpenGL texture abstraction
 *
 * The LTexture class is an abstraction of an OpenGL texture.
 * It provides a unified interface for generating and updating textures from buffers in main memory, `wl_drm` buffers, and DMA buffers.
 * The current texture buffer content is destroyed and replaced every time any of the setData() variants is used.
 *
 * @see To create a texture from an image file use LOpenGL::loadTexture().
 */
class Louvre::LTexture : public LObject
{
public:

    /**
     * @brief Texture source enumeration indicating the origin of the texture data.
     */
    enum BufferSourceType
    {
        /// Buffer sourced from main memory
        CPU = 0,

        /// Buffer sourced from `wl_drm` buffers
        WL_DRM = 1,

        /// Buffer sourced from DMA planes
        DMA = 2,

        /// Indicates the texture is from an LFramebuffer.
        Framebuffer = 3
    };

    /**
     * @brief Create an empty texture.
     */
    LTexture();

    /// @cond OMIT
    LTexture(const LTexture&) = delete;
    LTexture& operator= (const LTexture&) = delete;
    /// @endcond

    /**
     * @brief The LTexture class destructor.
     */
    ~LTexture();

    /**
     * @brief Get the equivalent DRM buffer format from a Wayland buffer format.
     *
     * DRM formats are listed in [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h).
     *
     * @param waylandFormat The Wayland buffer format to convert.
     * @return The equivalent DRM buffer format.
     */
    static UInt32 waylandFormatToDRM(UInt32 waylandFormat);

    /**
     * @brief Get the number of bytes (not bits) per pixel of a DRM format.
     *
     * @param format The DRM format to get the bytes per pixel for.
     * @return The number of bytes per pixel.
     */
    static UInt32 formatBytesPerPixel(UInt32 format);

    /**
     * @brief Set the data of the texture from a buffer in main memory.
     *
     * @param size The size of the texture in buffer coordinates.
     * @param stride The stride of the buffer.
     * @param format The DRM format of the buffer (defined in [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h)).
     * @param buffer The pointer to the main memory buffer.
     * @return `true` if the data was successfully set, `false` otherwise.
     */
    bool setDataB(const LSize &size, UInt32 stride, UInt32 format, const void *buffer);

    /**
     * @brief Set the data of the texture from a `wl_drm` buffer.
     *
     * @param wlDRMBuffer The pointer to the `wl_drm` buffer.
     * @return `true` if the data was successfully set, `false` otherwise.
     */
    bool setData(void *wlDRMBuffer);

    /**
     * @brief Set the data of the texture from DMA planes.
     *
     * @param planes The pointer to the DMA planes struct.
     * @return `true` if the data was successfully set, `false` otherwise.
     */
    bool setDataB(const LDMAPlanes *planes);

    /**
     * @brief Update a specific area of the texture with the provided buffer.
     *
     * To successfully update the texture, the provided buffer must have the same format as the texture.
     * If invalid parameters are passed or if the texture cannot be modified, this function returns `false`.
     *
     * @param rect The rect within the texture to update, specified in buffer coordinates with the top-left corner as the origin.
     * @param stride The stride of the main memory buffer.
     * @param buffer A pointer to the main memory buffer.
     * @return `true` if the update was successful; otherwise, `false`.
     */
    bool updateRect(const LRect &rect, UInt32 stride, const void *buffer);

    /**
     * @brief Create a copy of the texture.
     *
     * The destination size (dst) is the size of the copied texture, and the source (src) is the rectangular area within the texture to be copied.
     * Using both parameters, a clipped and scaled copy version can be created from the texture.\n
     * Using negative values for the source rect size (srcW and srcH) causes the texture to be mirrored along the given axis.
     *
     * @note The resulting texture is independent of the original and must be freed separately.
     *
     * @param dst The destination size of the copied texture. Passing an empty Louvre::LSize means the same texture size is used.
     * @param src The rectangular area within the texture to be copied. Passing an empty Louvre::LRect means the entire texture is used.
     * @param highQualityScaling Set this value to `true` to enable high-quality scaling, which produces better results when resizing to a significantly different size from the original.
     * @return A pointer to the copied LTexture object.
     */
    LTexture *copyB(const LSize &dst = LSize(), const LRect &src = LRect(), bool highQualityScaling = true) const;

    /**
     * @brief Save the texture as a PNG file.
     *
     * This function allows you to save the texture as a PNG image file at the specified @p path.
     *
     * @param path The file path where the PNG image will be saved.
     * @return `true` if the save operation is successful, `false` otherwise.
     */
    bool save(const char *path) const;

    /**
     * @brief Get the size of the texture in buffer coordinates.
     *
     * @return The size of the texture.
     */
    const LSize &sizeB() const;

    /**
     * @brief Check if the texture has been initialized.
     *
     * A texture is considered initialized when content has been assigned to it using any setData() variant.
     *
     * @return `true` if the texture has been initialized, `false` otherwise.
     */
    bool initialized() const;

    /**
     * @brief Get the OpenGL texture ID for a specific output.
     *
     * If nullptr is passed as output, a texture for the main thread is returned.
     *
     * @param output The specific output for which to get the texture ID.
     * @return The OpenGL texture ID or 0 if fails.
     */
    GLuint id(LOutput *output) const;

    /**
     * @brief Get the OpenGL texture target.
     *
     * @return The OpenGL texture target.
     */
    GLenum target() const;

    /**
     * @brief Get the texture source type.
     *
     * @return The texture source type as an LTexture::BufferSourceType enum value.
     */
    BufferSourceType sourceType() const;

    /**
     * @brief Get the DRM format of the texture.
     *
     * @see [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h) for more information on DRM formats.
     *
     * @return The DRM format of the texture.
     */
    UInt32 format() const;

    LPRIVATE_IMP(LTexture)
};

#endif
