#include <protocols/Wayland/private/RegionResourcePrivate.h>
#include <protocols/Wayland/CompositorGlobal.h>

#include <LRegion.h>
#include <LClient.h>
#include <stdio.h>
#include <LCompositor.h>

using namespace Louvre::Protocols::Wayland;

struct wl_region_interface region_implementation =
{
    .destroy = &RegionResource::RegionResourcePrivate::destroy,
    .add = &RegionResource::RegionResourcePrivate::add,
    .subtract = &RegionResource::RegionResourcePrivate::subtract
};

Protocols::Wayland::RegionResource::RegionResource(CompositorGlobal *compositorGlobal,
                                                   UInt32 id) :
    LResource(
        compositorGlobal->client(),
        &wl_region_interface,
        compositorGlobal->version(),
        id,
        &region_implementation,
        &RegionResource::RegionResourcePrivate::resource_destroy)
{
    m_imp = new RegionResourcePrivate();
}

RegionResource::~RegionResource()
{
    delete m_imp;
}

const LRegion &Protocols::Wayland::RegionResource::region() const
{
    return imp()->region;
}

RegionResource::RegionResourcePrivate *RegionResource::imp() const
{
    return m_imp;
}
