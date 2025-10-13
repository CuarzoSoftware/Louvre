#ifndef RIMAGECAPTURESOURCE_H
#define RIMAGECAPTURESOURCE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::ImageCaptureSource::RImageCaptureSource final : public LResource
{
public:

    LImageCaptureSourceType type() const noexcept
    {
        return m_type;
    }

    LResource *source() const noexcept
    {
        return m_source.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    friend class CZ::Protocols::ImageCaptureSource::GOutputImageCaptureSourceManager;
    friend class CZ::Protocols::ImageCaptureSource::GForeignToplevelImageCaptureSourceManager;

    RImageCaptureSource(LClient *client, Int32 version, UInt32 id, LImageCaptureSourceType type, LResource *source) noexcept;
    ~RImageCaptureSource() = default;

    CZWeak<LResource> m_source; // Wayland::GOutput or ForeignToplevelList::RForeignToplevelHandle
    LImageCaptureSourceType m_type;
};

#endif // RIMAGECAPTURESOURCE_H
