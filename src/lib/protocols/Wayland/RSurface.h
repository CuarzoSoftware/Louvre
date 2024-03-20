#ifndef RSURFACE_H
#define RSURFACE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RSurface final : public LResource
{
public:

    /**
     * @brief Commit origin
     * Indicates who requests to commit a surface
     */
    enum CommitOrigin
    {
        /// @brief The commit is requested by the surface itself
        Itself,

        /// @brief The commit is requested by the parent surface
        Parent
    };

    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

    Viewporter::RViewport *viewportRes() const noexcept
    {
        return m_viewportRes.get();
    }

    FractionalScale::RFractionalScale *fractionalScaleRes() const noexcept
    {
        return m_fractionalScaleRes.get();
    }

    TearingControl::RTearingControl *tearingControlRes() const noexcept
    {
        return m_tearingControlRes.get();
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
    static void apply_commit(LSurface *surface, CommitOrigin origin = Itself);

    /******************** EVENTS ********************/

    // Since 1
    void enter(GOutput *outputRes) noexcept;
    void leave(GOutput *outputRes) noexcept;

    // Since 6
    bool preferredBufferScale(Int32 scale) noexcept;
    bool preferredBufferTransform(UInt32 transform) noexcept;

private:
    friend class Louvre::Protocols::Wayland::GCompositor;
    friend class Louvre::Protocols::Viewporter::RViewport;
    friend class Louvre::Protocols::FractionalScale::RFractionalScale;
    friend class Louvre::Protocols::TearingControl::RTearingControl;
    RSurface(GCompositor *compositorRes, UInt32 id);
    ~RSurface();
    std::unique_ptr<LSurface> m_surface;
    LWeak<Viewporter::RViewport> m_viewportRes;
    LWeak<FractionalScale::RFractionalScale> m_fractionalScaleRes;
    LWeak<TearingControl::RTearingControl> m_tearingControlRes;
};

#endif // RSURFACE_H
