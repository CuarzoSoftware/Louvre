#ifndef CZ_RWLSURFACE_H
#define CZ_RWLSURFACE_H

#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZTransform.h>
#include <memory>

class CZ::Protocols::Wayland::RWlSurface final : public LResource
{
public:

    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

    Viewporter::RViewport *viewportRes() const noexcept
    {
        return m_viewportRes;
    }

    FractionalScale::RFractionalScale *fractionalScaleRes() const noexcept
    {
        return m_fractionalScaleRes;
    }

    TearingControl::RTearingControl *tearingControlRes() const noexcept
    {
        return m_tearingControlRes;
    }

    ContentType::RWpContentTypeV1 *contentTypeRes() const noexcept
    {
        return m_contentTypeRes;
    }

    /******************** REQUESTS ********************/

    static void attach(wl_client *client, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y);
    static void frame(wl_client *client, wl_resource *resource, UInt32 callback);
    static void destroy(wl_client *client, wl_resource *resource);
    static void commit(wl_client *client, wl_resource *resource);
    static void damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height);
    static void set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region);
    static void set_input_region(wl_client *client, wl_resource *resource, wl_resource *region);

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
    static void set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform);
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
    static void set_buffer_scale(wl_client *client, wl_resource *resource, Int32 scale);
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
    static void damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 w, Int32 h);
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    static void offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y);
#endif

    static void handleOffset(LSurface *lSurface, Int32 x, Int32 y);

    /******************** EVENTS ********************/

    // Since 1
    void enter(GOutput *outputRes) noexcept;
    void leave(GOutput *outputRes) noexcept;

    // Since 6
    bool preferredBufferScale(Int32 scale) noexcept;
    bool preferredBufferTransform(CZTransform transform) noexcept;

private:
    friend class CZ::Protocols::Wayland::GCompositor;
    friend class CZ::Protocols::Viewporter::RViewport;
    friend class CZ::Protocols::FractionalScale::RFractionalScale;
    friend class CZ::Protocols::TearingControl::RTearingControl;
    friend class CZ::Protocols::ContentType::RWpContentTypeV1;
    RWlSurface(GCompositor *compositorRes, UInt32 id);
    ~RWlSurface();
    std::unique_ptr<LSurface> m_surface;
    CZWeak<Viewporter::RViewport> m_viewportRes;
    CZWeak<FractionalScale::RFractionalScale> m_fractionalScaleRes;
    CZWeak<TearingControl::RTearingControl> m_tearingControlRes;
    CZWeak<ContentType::RWpContentTypeV1> m_contentTypeRes;
};

#endif // CZ_RWLSURFACE_H
