#ifndef LSINGLEPIXELBUFFER_H
#define LSINGLEPIXELBUFFER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Ream/RImage.h>

class CZ::LSinglePixelBuffer final : public LResource
{
public:
    std::shared_ptr<RImage> image;

    static bool isSinglePixelBuffer(wl_resource *buffer) noexcept;

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);

private:
    friend class Protocols::SinglePixelBuffer::GWpSinglePixelBufferManagerV1;
    LSinglePixelBuffer(LClient *client, Int32 version, UInt32 id, std::shared_ptr<RImage> image) noexcept;
    ~LSinglePixelBuffer() noexcept = default;
};

#endif // LSINGLEPIXELBUFFER_H
