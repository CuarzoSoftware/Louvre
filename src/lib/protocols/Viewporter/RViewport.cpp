#include <protocols/Viewporter/private/RViewportPrivate.h>
#include <protocols/Viewporter/viewporter.h>
#include <protocols/Wayland/RSurface.h>

using namespace Louvre;

static struct wp_viewport_interface viewport_implementation =
{
    .destroy = &RViewport::RViewportPrivate::destroy,
    .set_source = &RViewport::RViewportPrivate::set_source,
    .set_destination = &RViewport::RViewportPrivate::set_destination
};

RViewport::RViewport
    (
        Wayland::RSurface *rSurface,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        rSurface->client(),
        &wp_viewport_interface,
        version,
        id,
        &viewport_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RViewport)
{
    imp()->rSurface.reset(rSurface);
    rSurface->m_viewportRes.reset(this);
}

RViewport::~RViewport() {}

Protocols::Wayland::RSurface *RViewport::surfaceResource() const
{
    return imp()->rSurface.get();
}

const LSize &RViewport::dstSize() const
{
    return imp()->dstSize;
}

const LRectF &RViewport::srcRect() const
{
    return imp()->srcRect;
}
