#include <protocols/Viewporter/private/RViewportPrivate.h>
#include <protocols/Viewporter/viewporter.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>

using namespace Louvre;

static struct wp_viewport_interface viewport_implementation =
{
    .destroy = &RViewport::RViewportPrivate::destroy,
    .set_source = &RViewport::RViewportPrivate::set_source,
    .set_destination = &RViewport::RViewportPrivate::set_destination
};

RViewport::RViewport
    (
        RSurface *rSurface,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        rSurface->client(),
        &wp_viewport_interface,
        version,
        id,
        &viewport_implementation,
        &RViewport::RViewportPrivate::resource_destroy
    ),
    LPRIVATE_INIT_UNIQUE(RViewport)
{
    imp()->rSurface = rSurface;
    rSurface->imp()->rViewport = this;
}

RViewport::~RViewport()
{
    if (imp()->rSurface)
        imp()->rSurface->imp()->rViewport = nullptr;
}

RSurface *RViewport::surfaceResource() const
{
    return imp()->rSurface;
}

const LSize &RViewport::dstSize() const
{
    return imp()->dstSize;
}

const LRectF &RViewport::srcRect() const
{
    return imp()->srcRect;
}
