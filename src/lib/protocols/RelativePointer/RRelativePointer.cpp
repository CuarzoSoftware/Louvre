#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <protocols/RelativePointer/RRelativePointer.h>
#include <protocols/Wayland/RPointer.h>
#include <LPointerMoveEvent.h>
#include <LUtils.h>

using namespace Louvre::Protocols::RelativePointer;

static const struct zwp_relative_pointer_v1_interface imp
{
    .destroy = &RRelativePointer::destroy
};

RRelativePointer::RRelativePointer(Wayland::RPointer *pointerRes, Int32 id, UInt32 version) noexcept :
    LResource(
        pointerRes->client(),
        &zwp_relative_pointer_v1_interface,
        version,
        id,
        &imp),
    m_pointerRes(pointerRes)
{
    pointerRes->m_relativePointerRes.emplace_back(this);
}

RRelativePointer::~RRelativePointer() noexcept
{
    if (pointerRes())
        LVectorRemoveOneUnordered(pointerRes()->m_relativePointerRes, this);
}

void RRelativePointer::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RRelativePointer::relativeMotion(const Louvre::LPointerMoveEvent &event) noexcept
{
    zwp_relative_pointer_v1_send_relative_motion(
        resource(),
        event.us() >> 32,
        event.us() & 0xffffffff,
        wl_fixed_from_double(event.delta().x()),
        wl_fixed_from_double(event.delta().y()),
        wl_fixed_from_double(event.deltaUnaccelerated().x()),
        wl_fixed_from_double(event.deltaUnaccelerated().y()));
}
