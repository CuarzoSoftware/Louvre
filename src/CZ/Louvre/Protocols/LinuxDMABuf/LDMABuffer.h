#ifndef LDMABUFFER_H
#define LDMABUFFER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Ream/RImage.h>

using namespace CZ::Protocols::LinuxDMABuf;

class CZ::LDMABuffer final : public LResource
{
public:
    LDMABuffer(std::shared_ptr<RImage> &&image, LClient *client, UInt32 id) noexcept;

    static bool isDMABuffer(wl_resource *buffer) noexcept;

    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    ~LDMABuffer() noexcept;
    std::shared_ptr<RImage> m_image;
};

#endif // LDMABUFFER_H
