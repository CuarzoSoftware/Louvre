#ifndef RIMAGECAPTURESOURCE_H
#define RIMAGECAPTURESOURCE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Seat/LOutput.h>

class CZ::Protocols::ImageCaptureSource::RImageCaptureSource final : public LResource
{
public:

    LImageCaptureSourceType type() const noexcept
    {
        return m_type;
    }

    /// The captured output (Output sources, nullptr once gone)
    LOutput *output() const noexcept { return m_output.get(); }

    /// The captured toplevel (ForeignToplevel sources, nullptr once gone)
    LToplevelRole *toplevel() const noexcept { return m_toplevel.get(); }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

private:
    friend class CZ::Protocols::ImageCaptureSource::GOutputImageCaptureSourceManager;
    friend class CZ::Protocols::ImageCaptureSource::GForeignToplevelImageCaptureSourceManager;

    RImageCaptureSource(LClient *client, Int32 version, UInt32 id, LImageCaptureSourceType type, LOutput *output, LToplevelRole *toplevel) noexcept;
    ~RImageCaptureSource() = default;

    CZWeak<LOutput> m_output;
    CZWeak<LToplevelRole> m_toplevel;
    LImageCaptureSourceType m_type;
};

#endif // RIMAGECAPTURESOURCE_H
