#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <LObject.h>
#include <LRect.h>
#include <LWeak.h>

#include <filesystem>
#include <drm_fourcc.h>
#include <GL/gl.h>

#define LOUVRE_MAX_DMA_PLANES 4

#if LOUVRE_USE_SKIA == 1
#include <include/core/SkImage.h>
#endif

namespace Louvre
{
    /**
     * @brief Structure representing a DMA format and modifier.
     *
     * The LDMAFormat struct contains information about a DMA format and modifier.\n
     * It is used to describe the format and memory layout of DMA planes used for texture generation.\n
     */
    struct LDMAFormat
    {
        /// The DRM format of the DMA plane.
        UInt32 format;

        /// The DRM modifier value specifying the memory layout.
        UInt64 modifier;

        bool operator==(const LDMAFormat &other) const noexcept
        {
            return format == other.format && modifier == other.modifier;
        }
    };

    /**
     * @brief Direct Memory Access (DMA) planes.
     *
     * Use this struct to import DMA buffers with LTexture.
     */
    struct LDMAPlanes
    {
        /// Width of the buffer in pixels.
        UInt32 width;

        /// Height of the buffer in pixels.
        UInt32 height;

        /// DRM format of the buffer.
        UInt32 format;

        /// Number of file descriptors.
        UInt32 num_fds = 0;

        /// Array of file descriptors associated with each DMA plane.
        Int32 fds[LOUVRE_MAX_DMA_PLANES] = {-1};

        /// Array of strides for each DMA plane.
        UInt32 strides[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of offsets for each DMA plane.
        UInt32 offsets[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of modifiers for each DMA plane.
        UInt64 modifiers[LOUVRE_MAX_DMA_PLANES] = {0};
    };

    /**
     * @brief OpenGL texture abstraction
     *
     * The LTexture class is an abstraction of an OpenGL texture.\n
     * It provides a unified interface for generating and updating textures from buffers in main memory, `wl_drm` buffers, and DMA buffers.
     *
     * @warning The texture internal buffer storage is destroyed and replaced every time any of the setData() variants is used.
     *
     * @see To create a texture from an image file use LOpenGL::loadTexture().
     */
    class LTexture final : public LObject
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
            Framebuffer = 3,

            /// Sourced from a native OpenGL ES 2.0 texture
            GL = 4
        };

        /**
         * @brief Creates an empty texture.
         */
        LTexture(bool premultipliedAlpha = false) noexcept;

        LCLASS_NO_COPY(LTexture)

        /**
         * @brief The LTexture class destructor.
         */
        ~LTexture() noexcept;

        /**
         * @brief Gets the equivalent DRM buffer format from a Wayland SHM buffer format.
         *
         * DRM formats are listed in [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h).
         *
         * @param waylandFormat The Wayland buffer format to convert.
         * @return The equivalent DRM buffer format.
         */
        static UInt32 waylandFormatToDRM(UInt32 waylandFormat) noexcept;

        /**
         * @brief Gets the number of bytes (not bits) per pixel of a DRM format.
         *
         * @param format The DRM format to get the bytes per pixel for.
         * @return The number of bytes per pixel.
         */
        static UInt32 formatBytesPerPixel(UInt32 format) noexcept;

        /**
         * @brief Gets the number of planes of a DRM format.
         *
         * @param format The DRM format.
         * @return The number of planes of the format.
         */
        static UInt32 formatPlanes(UInt32 format) noexcept;

        /**
         * @brief Retrieves the DMA formats supported by the graphics backend.
         */
        static const std::vector<LDMAFormat> &supportedDMAFormats() noexcept;

        /**
         * @brief Set the data of the texture from a buffer in main memory.
         *
         * @param size Width and height of the source buffer.
         * @param stride The stride of the source buffer.
         * @param format The DRM format of the buffer (defined in [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h)).
         * @param buffer The pointer to the source main memory buffer.
         * @return `true` if the data was successfully set, `false` otherwise.
         */
        bool setDataFromMainMemory(const LSize &size, UInt32 stride, UInt32 format, const void *buffer) noexcept;

        /**
         * @brief Set the data of the texture from a `wl_drm` buffer.
         *
         * @param buffer The pointer to the `wl_drm` buffer.
         * @return `true` if the data was successfully set, `false` otherwise.
         */
        bool setDataFromWaylandDRM(wl_resource *buffer) noexcept;

        /**
         * @brief Set the data of the texture from DMA planes.
         *
         * @param planes The pointer to the DMA planes struct.
         * @return `true` if the data was successfully set, `false` otherwise.
         */
        bool setDataFromDMA(const LDMAPlanes &planes) noexcept;

        /**
         * @brief Creates a wrapper around an existing OpenGL texture.
         *
         * @param id The OpenGL texture ID.
         * @param target The OpenGL target for the texture (e.g., GL_TEXTURE_2D).
         * @param format The format of the texture data (specified as a DRM format).
         * @param size The dimensions of the texture in pixels.
         * @param transferOwnership If set to `true`, the texture will be automatically destroyed when the LTexture is destroyed or replaced.
         *
         * @return `true` if the texture data was successfully set, `false` otherwise.
         */
        bool setDataFromGL(GLuint id, GLenum target, UInt32 format, const LSize &size, bool transferOwnership) noexcept;

        /**
         * @brief Update a specific area of the texture with the provided buffer.
         *
         * The provided buffer must have the same format as the texture.\n
         * If invalid parameters are passed or if the texture cannot be modified, this method returns `false`.
         *
         * @param rect The rect within the texture to update, specified in buffer coordinates with the top-left corner as the origin.
         * @param stride The stride of the source main memory buffer.
         * @param buffer A pointer to the origin of the source main memory buffer.
         * @return `true` if the update was successful; otherwise, `false`.
         */
        bool updateRect(const LRect &rect, UInt32 stride, const void *buffer) noexcept;

        /**
         * @brief Creates a copy of the texture.
         *
         * @note The resulting texture is independent of the original and must be freed manually when no longer used.
         *
         * @param dst The destination size of the copied texture. Pass (0,0) to use the same size as the original texture.
         * @param src The rectangular area within the source texture to be copied. Pass (0,0,0,0) to copy the entire texture.
         * @param highQualityScaling Set this parameter to `true` to enable high-quality scaling, which produces better results when resizing to a significantly different size from the original.
         *
         * @return A pointer to the copied LTexture object or `nullptr` on failure.
         */
        LTexture *copy(const LSize &dst = LSize(), const LRect &src = LRect(), bool highQualityScaling = true) const noexcept;

        /**
         * @brief Save the texture as a PNG file.
         *
         * This method allows you to save the texture as a PNG image file at the specified @p name.
         *
         * @param name Full path and file name where the PNG image will be saved.
         * @return `true` if the save operation is successful, `false` otherwise.
         */
        bool save(const std::filesystem::path &name) const noexcept;

        /**
         * @brief Sets a synchronization fence.
         *
         * This method should be called after rendering is performed into the texture
         * or the pixel data is updated via functions not defined within LTexture.
         */
        void setFence() noexcept;

        /**
         * @brief Gets the size of the texture in buffer coordinates.
         *
         * @return The size of the texture.
         */
        const LSize &sizeB() const noexcept
        {
            return m_sizeB;
        }

        /**
         * @brief Check if the texture has been initialized.
         *
         * A texture is considered initialized when content has been assigned to it using any of the **setDataXXX()** variants.
         *
         * @return `true` if the texture has been initialized, `false` otherwise.
         */
        bool initialized() const noexcept
        {
            return m_graphicBackendData != nullptr;
        }

        /**
         * @brief Gets the OpenGL texture ID for a specific output.
         *
         * If `nullptr` is passed as output, a texture for the main thread is returned.
         *
         * @param output The specific output for which to get the texture ID.
         * @return The OpenGL texture ID or 0 if fails.
         */
        GLuint id(LOutput *output) const noexcept;

        /**
         * @brief Gets the OpenGL texture target.
         *
         * @return The OpenGL texture target.
         */
        GLenum target() const noexcept
        {
            if (initialized())
            {
                if (sourceType() == Framebuffer)
                    return GL_TEXTURE_2D;

                return backendTarget();
            }

            return GL_TEXTURE_2D;
        }

        /**
         * @brief Gets the texture source type.
         *
         * @return The texture source type as an LTexture::BufferSourceType enum value.
         */
        BufferSourceType sourceType() const noexcept
        {
            return m_sourceType;
        }

        /**
         * @brief Gets the DRM format of the texture.
         *
         * @see [`drm_fourcc.h`](https://github.com/torvalds/linux/blob/master/include/uapi/drm/drm_fourcc.h) for more information on DRM formats.
         *
         * @return The DRM format of the texture.
         */
        UInt32 format() const noexcept
        {
            return m_format;
        }

        /**
         * @brief Gets the serial number of the texture.
         *
         * The serial number is incremented each time the texture's backing storage, or its pixel data changes.
         *
         * @return The serial number of the texture.
         */
        UInt32 serial() const noexcept
        {
            return m_serial;
        }

        /**
         * @brief Indicates whether the RGB components are pre-multiplied by the alpha component.
         *
         * This flag is crucial for proper blending of textures within LPainter and LScene.\n
         * By default, Wayland clients typically use pre-multiplied alpha, but user-defined textures may not adhere to this convention.
         *
         * To modify this value, use setPremultipliedAlpha().
         *
         * @return `true` if the RGB components are pre-multiplied by the alpha component, `false` otherwise.
         */
        bool premultipliedAlpha() const noexcept
        {
            return m_premultipliedAlpha;
        }

        /**
         * @brief Sets a hint indicating whether the RGB components are pre-multiplied by the alpha component.
         *
         * This function allows you to modify the pre-multiplied alpha flag.
         *
         * @param premultipledAlpha `true` to indicate pre-multiplied alpha, `false` otherwise.
         */
        void setPremultipliedAlpha(bool premultipledAlpha) const noexcept
        {
            m_premultipliedAlpha = premultipledAlpha;
        }

#if LOUVRE_USE_SKIA == 1
        const sk_sp<SkImage> skImage() const noexcept;
#endif
        class LTexturePrivate;

    private:
        friend class LCompositor;
        friend class LRenderBuffer;
        friend class LGraphicBackend;
        friend class LDMABuffer;
        friend class LSurface;
        friend class LOutput;

        void *m_graphicBackendData { nullptr };
        LSize m_sizeB;
        UInt32 m_format { 0 };
        BufferSourceType m_sourceType { CPU };
        UInt32 m_serial { 0 };
        bool m_pendingDelete { false };
        mutable bool m_premultipliedAlpha;

        LWeak<LSurface> m_surface;
        GLenum backendTarget() const noexcept;
        void reset() noexcept;

#if LOUVRE_USE_SKIA == 1
        mutable sk_sp<SkImage> m_skImage;
        mutable UInt32 m_skSerial { 0 };
#endif
    };
};

#endif
