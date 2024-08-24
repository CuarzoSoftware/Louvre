#ifndef RIMAGECAPTURESOURCE_H
#define RIMAGECAPTURESOURCE_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::ImageCaptureSource::RImageCaptureSource final : public LResource
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
    friend class Louvre::Protocols::ImageCaptureSource::GOutputImageCaptureSourceManager;
    friend class Louvre::Protocols::ImageCaptureSource::GForeignToplevelImageCaptureSourceManager;

    RImageCaptureSource(LClient *client, Int32 version, UInt32 id, LImageCaptureSourceType type, LResource *source) noexcept;
    ~RImageCaptureSource() = default;

    LWeak<LResource> m_source; // Wayland::GOutput or ForeignToplevelList::RForeignToplevelHandle
    LImageCaptureSourceType m_type;
};

#endif // RIMAGECAPTURESOURCE_H
