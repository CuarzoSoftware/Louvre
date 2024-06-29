#ifndef RSURFACE_H
#define RSURFACE_H

#include <LBaseSurfaceRole.h>
#include <LResource.h>
#include <LTransform.h>
#include <memory>

class Louvre::Protocols::Wayland::RSurface final : public LResource
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

    ContentType::RContentType *contentTypeRes() const noexcept
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
    static void apply_commit(LSurface *surface, LBaseSurfaceRole::CommitOrigin origin = LBaseSurfaceRole::CommitOrigin::Itself);

    /******************** EVENTS ********************/

    // Since 1
    void enter(GOutput *outputRes) noexcept;
    void leave(GOutput *outputRes) noexcept;

    // Since 6
    bool preferredBufferScale(Int32 scale) noexcept;
    bool preferredBufferTransform(LTransform transform) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GCompositor;
    friend class Louvre::Protocols::Viewporter::RViewport;
    friend class Louvre::Protocols::FractionalScale::RFractionalScale;
    friend class Louvre::Protocols::TearingControl::RTearingControl;
    friend class Louvre::Protocols::ContentType::RContentType;
    RSurface(GCompositor *compositorRes, UInt32 id);
    ~RSurface();
    std::unique_ptr<LSurface> m_surface;
    LWeak<Viewporter::RViewport> m_viewportRes;
    LWeak<FractionalScale::RFractionalScale> m_fractionalScaleRes;
    LWeak<TearingControl::RTearingControl> m_tearingControlRes;
    LWeak<ContentType::RContentType> m_contentTypeRes;
};

#endif // RSURFACE_H
