#ifndef LSINGLEPIXELBUFFER_H
#define LSINGLEPIXELBUFFER_H

#include <LResource.h>

class Louvre::LSinglePixelBuffer final : public LResource
{
public:

    struct UPixel32
    {
        UInt32 r { 0 };
        UInt32 g { 0 };
        UInt32 b { 0 };
        UInt32 a { 0 };
    };

    const UPixel32 &pixel() const noexcept
    {
        return m_pixel;
    }

    static bool isSinglePixelBuffer(wl_resource *buffer) noexcept;

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);

private:
    friend class Protocols::SinglePixelBuffer::GSinglePixelBufferManager;
    UPixel32 m_pixel;
    LSinglePixelBuffer(LClient *client, Int32 version, UInt32 id, const UPixel32 &pixel) noexcept;
    ~LSinglePixelBuffer() noexcept = default;
};

#endif // LSINGLEPIXELBUFFER_H
