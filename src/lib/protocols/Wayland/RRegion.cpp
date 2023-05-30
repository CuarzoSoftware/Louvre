#include <protocols/Wayland/private/RRegionPrivate.h>
#include <protocols/Wayland/GCompositor.h>

#include <LRegion.h>
#include <LClient.h>
#include <stdio.h>
#include <LCompositor.h>

using namespace Louvre::Protocols::Wayland;

struct wl_region_interface region_implementation =
{
    .destroy = &RRegion::RRegionPrivate::destroy,
    .add = &RRegion::RRegionPrivate::add,
    .subtract = &RRegion::RRegionPrivate::subtract
};

RRegion::RRegion
(
    GCompositor *compositorGlobal,
    UInt32 id
)
    :LResource
    (
        compositorGlobal->client(),
        &wl_region_interface,
        compositorGlobal->version(),
        id,
        &region_implementation,
        &RRegion::RRegionPrivate::resource_destroy
    )
{
    m_imp = new RRegionPrivate();
}

RRegion::~RRegion()
{
    delete m_imp;
}

const LRegion &RRegion::region() const
{
    return imp()->region;
}
