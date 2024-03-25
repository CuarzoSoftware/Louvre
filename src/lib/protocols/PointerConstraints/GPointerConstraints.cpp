#include <protocols/PointerConstraints/GPointerConstraints.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::PointerConstraints;

static const struct zwp_pointer_constraints_v1_interface imp
{
    .destroy = &GPointerConstraints::destroy,
    .lock_pointer = &GPointerConstraints::lock_pointer,
    .confine_pointer = &GPointerConstraints::confine_pointer
};

GPointerConstraints::GPointerConstraints(
    wl_client *client,
    Int32 version,
    UInt32 id
    ) noexcept
    :LResource
    (
        client,
        &zwp_pointer_constraints_v1_interface,
        version,
        id,
        &imp
    )
{
    this->client()->imp()->pointerConstraintsGlobals.emplace_back(this);
}

GPointerConstraints::~GPointerConstraints() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->pointerConstraintsGlobals, this);
}

/******************** REQUESTS ********************/

void GPointerConstraints::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GPointerConstraints(client, version, id);
}

void GPointerConstraints::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GPointerConstraints::lock_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *pointer, wl_resource *region, UInt32 lifetime) noexcept
{

}

void GPointerConstraints::confine_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *pointer, wl_resource *region, UInt32 lifetime) noexcept
{

}
