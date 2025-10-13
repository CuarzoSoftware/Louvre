#include <CZ/Louvre/Protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <CZ/Louvre/Protocols/RelativePointer/RRelativePointer.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::RelativePointer;

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
        CZVectorUtils::RemoveOneUnordered(pointerRes()->m_relativePointerRes, this);
}

void RRelativePointer::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RRelativePointer::relativeMotion(const CZPointerMoveEvent &event) noexcept
{
    zwp_relative_pointer_v1_send_relative_motion(
        resource(),
        event.us >> 32,
        event.us & 0xffffffff,
        wl_fixed_from_double(event.delta.x()),
        wl_fixed_from_double(event.delta.y()),
        wl_fixed_from_double(event.deltaUnaccelerated.x()),
        wl_fixed_from_double(event.deltaUnaccelerated.y()));
}
